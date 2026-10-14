package main

import (
	"context"
	"crypto/sha256"
	"encoding/hex"
	"errors"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"path/filepath"
	"strings"

	"rom_pack/compress"
	"rom_pack/rom_pack_db"
)

/*
1. Accept  POST /upload  as  multipart/form-data  with metadata and the file.
2. Get the authenticated user ID from the auth middleware.
3. Stream the uploaded file into a server-created temporary file:
• enforce a maximum size;
• calculate the checksum while writing;
• never trust the client-provided filename or path.
4. Pass the completed temporary file to  compress.Compress(...) .
5. Write the compressed result to another temporary file.
6. Atomically rename the finished file into its final server-generated path.
7. Insert the  games / blobs / game_categories  records in a transaction.
8. Delete temporary files on success or failure.
*/
const (
	maxUploadSize   = 4 << 30
	maxMetadataSize = 64 << 10
	uploadRoot      = "uploads"
)

type UploadMetadata struct {
	GameName     string
	GameSystem   string
	CategorySlug string
}

type userIDContextKey struct{}

func userIDFromContext(ctx context.Context) (int, bool) {
	userID, ok := ctx.Value(userIDContextKey{}).(int)
	return userID, ok && userID > 0
}

func HandleFileUpload(store *rom_pack_db.Store) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		if r.Method != http.MethodPost {
			writeAPIError(w, http.StatusMethodNotAllowed, "Method not allowed")
			return
		}

		userID, ok := userIDFromContext(r.Context())
		if !ok {
			writeAPIError(w, http.StatusUnauthorized, "Authentication required")
			return
		}

		r.Body = http.MaxBytesReader(w, r.Body, maxUploadSize+maxMetadataSize)
		if err := r.ParseMultipartForm(maxMetadataSize); err != nil {
			writeAPIError(w, http.StatusBadRequest, "Invalid multipart form")
			return
		}
		defer r.MultipartForm.RemoveAll()

		metadata, err := parseUploadMetadata(r)
		if err != nil {
			writeAPIError(w, http.StatusBadRequest, err.Error())
			return
		}

		source, _, err := r.FormFile("file")
		if err != nil {
			writeAPIError(w, http.StatusBadRequest, "file is required")
			return
		}
		defer source.Close()

		inputPath, checksum, size, err := writeUploadTemp(source)
		if err != nil {
			writeAPIError(w, http.StatusBadRequest, err.Error())
			return
		}
		defer os.Remove(inputPath)

		compressedPath, compressFormat, err := compress.Compress(inputPath, metadata.CategorySlug)
		if err != nil {
			log.Printf("compress upload: user_id=%d error=%v", userID, err)
			writeAPIError(w, http.StatusInternalServerError, "Unable to compress file")
			return
		}
		if compressedPath == "" {
			writeAPIError(w, http.StatusInternalServerError, "Unable to compress file")
			return
		}
		defer os.Remove(compressedPath)

		finalPath := filepath.Join(uploadRoot, fmt.Sprintf("%d-%s.bin", userID, checksum))
		if err := os.MkdirAll(filepath.Dir(finalPath), 0o750); err != nil {
			writeAPIError(w, http.StatusInternalServerError, "Unable to create storage directory")
			return
		}
		if err := os.Rename(compressedPath, finalPath); err != nil {
			writeAPIError(w, http.StatusInternalServerError, "Unable to store compressed file")
			return
		}

		committed := false
		defer func() {
			if !committed {
				_ = os.Remove(finalPath)
			}
		}()

		if err := rom_pack_db.InsertUploadRecords(
			r.Context(),
			store,
			userID,
			metadata.GameName,
			metadata.GameSystem,
			metadata.CategorySlug,
			size,
			checksum,
			finalPath,
			compressFormat,
		); err != nil {
			log.Printf("save upload metadata: user_id=%d error=%v", userID, err)
			writeAPIError(w, http.StatusInternalServerError, "Unable to save upload metadata")
			return
		}

		committed = true
		writeJSON(w, http.StatusCreated, struct {
			Checksum string `json:"checksum"`
		}{Checksum: checksum})
	}
}

func parseUploadMetadata(r *http.Request) (UploadMetadata, error) {
	metadata := UploadMetadata{
		GameName:     strings.TrimSpace(r.FormValue("gamename")),
		GameSystem:   strings.TrimSpace(r.FormValue("game_system")),
		CategorySlug: strings.TrimSpace(r.FormValue("category")),
	}
	if metadata.GameName == "" || metadata.GameSystem == "" || metadata.CategorySlug == "" {
		return UploadMetadata{}, errors.New("gamename, game_system, and category are required")
	}
	return metadata, nil
}

func writeUploadTemp(source io.Reader) (string, string, int64, error) {
	temp, err := os.CreateTemp("", "rom-pack-upload-*")
	if err != nil {
		return "", "", 0, err
	}
	path := temp.Name()
	cleanup := true
	defer func() {
		_ = temp.Close()
		if cleanup {
			_ = os.Remove(path)
		}
	}()

	hash := sha256.New()
	size, err := io.Copy(temp, io.LimitReader(io.TeeReader(source, hash), maxUploadSize+1))
	if err != nil {
		return "", "", 0, err
	}
	if size > maxUploadSize {
		return "", "", 0, errors.New("file exceeds maximum size")
	}
	if err := temp.Sync(); err != nil {
		return "", "", 0, err
	}

	cleanup = false
	return path, hex.EncodeToString(hash.Sum(nil)), size, nil
}
