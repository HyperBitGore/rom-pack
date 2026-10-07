package rom_pack_db

import (
	"crypto/rand"
	"crypto/sha256"
	"encoding/hex"
)

func GetUserToken(store *Store, token string) (int, error) {
	tokenHash := sha256.Sum256([]byte(token))

	var userID int
	err := store.DB.QueryRow(
		"SELECT user_id FROM tokens WHERE token_hash = ? AND expires_at > CURRENT_TIMESTAMP",
		hex.EncodeToString(tokenHash[:]),
	).Scan(&userID)
	return userID, err
}

func DeleteToken(store *Store, tokenID int) error {
	_, err := store.DB.Exec("DELETE FROM tokens WHERE id = ?", tokenID)
	return err
}

func AddUserToken(store *Store, userID int) (string, error) {
	rawToken := make([]byte, 32)
	if _, err := rand.Read(rawToken); err != nil {
		return "", err
	}

	token := hex.EncodeToString(rawToken)
	tokenHash := sha256.Sum256([]byte(token))
	_, err := store.DB.Exec(
		"INSERT INTO tokens (user_id, token_hash, expires_at) VALUES (?, ?, datetime('now', '+7 days'))",
		userID,
		hex.EncodeToString(tokenHash[:]),
	)
	if err != nil {
		return "", err
	}
	return token, nil
}
