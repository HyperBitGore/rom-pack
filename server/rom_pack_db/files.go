package rom_pack_db

import (
	"context"
)

func InsertUploadRecords(
	ctx context.Context,
	store *Store,
	userID int,
	gameName string,
	gameSystem string,
	categorySlug string,
	size int64,
	checksum string,
	filePath string,
	compressionFormat string,
) error {
	tx, err := store.DB.BeginTx(ctx, nil)
	if err != nil {
		return err
	}
	defer tx.Rollback()

	result, err := tx.ExecContext(ctx,
		`INSERT INTO games (user_id, game_name, game_system, game_size, checksum)
		 VALUES (?, ?, ?, ?, ?)`,
		userID, gameName, gameSystem, size, checksum,
	)
	if err != nil {
		return err
	}
	gameID, err := result.LastInsertId()
	if err != nil {
		return err
	}

	_, err = tx.ExecContext(ctx,
		`INSERT INTO categories (user_id, name, slug) VALUES (?, ?, ?)
		 ON CONFLICT(user_id, slug) DO NOTHING`,
		userID, categorySlug, categorySlug,
	)
	if err != nil {
		return err
	}
	var categoryID int64
	if err := tx.QueryRowContext(ctx,
		`SELECT id FROM categories WHERE user_id = ? AND slug = ?`,
		userID, categorySlug,
	).Scan(&categoryID); err != nil {
		return err
	}

	if _, err := tx.ExecContext(ctx,
		`INSERT INTO game_categories (game_id, category_id) VALUES (?, ?)`,
		gameID, categoryID,
	); err != nil {
		return err
	}
	if _, err := tx.ExecContext(ctx,
		`INSERT INTO blobs (game_id, file_path, compression_format)
		 VALUES (?, ?, ?)`,
		gameID, filePath, compressionFormat,
	); err != nil {
		return err
	}

	return tx.Commit()
}
