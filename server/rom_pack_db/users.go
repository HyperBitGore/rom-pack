package rom_pack_db

import (
	"context"
	"database/sql"
	"log"
	"time"

	"golang.org/x/crypto/bcrypt"
)

// do hashing in this function
func AddUser(store *Store, username string, password string) error {
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()
	passwordHash, err := bcrypt.GenerateFromPassword([]byte(password), bcrypt.DefaultCost)
	if err != nil {
		return err
	}
	result, err := store.DB.ExecContext(
		ctx,
		"INSERT INTO users (username, password_hash) VALUES (?, ?)",
		username, passwordHash)
	if err != nil {
		return err
	}

	// Optional: Check rows affected
	rows, err := result.RowsAffected()
	if err == nil {
		log.Printf("Successfully inserted %d row(s)", rows)
	}

	return nil
}

func RemoveUser(store *Store, id int) error {
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()
	result, err := store.DB.ExecContext(
		ctx,
		"DELETE FROM users WHERE id = ?",
		id)
	if err != nil {
		return err
	}

	// Optional: Check rows affected
	rows, err := result.RowsAffected()
	if err == nil {
		log.Printf("Successfully inserted %d row(s)", rows)
	}

	return nil
}

func GetUsers(store *Store) (*sql.Rows, error) {
	return store.DB.QueryContext(
		context.Background(),
		"SELECT id, username, password_hash, created_at FROM users",
	)
}

func UserExists(store *Store, username string) (bool, error) {
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()

	var count int
	err := store.DB.QueryRowContext(
		ctx,
		"SELECT COUNT(*) FROM users WHERE username = ?",
		username,
	).Scan(&count)
	if err != nil {
		return false, err
	}

	return count > 0, nil
}

func GetUserID(store *Store, username string) (int, error) {
	var userID int
	err := store.DB.QueryRow(
		"SELECT id FROM users WHERE username = ?",
		username,
	).Scan(&userID)
	return userID, err
}

func UpdateUser(store *Store, id int) error {

	return nil
}

func UserLogin(store *Store, username string, password string) error {
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()

	var passwordHash []byte
	err := store.DB.QueryRowContext(
		ctx,
		"SELECT password_hash FROM users WHERE username = ?",
		username,
	).Scan(&passwordHash)
	if err != nil {
		return err
	}

	return bcrypt.CompareHashAndPassword(passwordHash, []byte(password))
}
