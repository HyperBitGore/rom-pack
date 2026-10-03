package rom_pack_db

import (
	"context"
	"database/sql"
	"fmt"
	"log"
	"time"

	"golang.org/x/crypto/bcrypt"
)

type Store struct {
	DB *sql.DB
}

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
		"INSERT INTO users (username, password) VALUES (?, ?)",
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
		"SELECT id, username, password, created_at FROM users",
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

func UpdateUser(store *Store, id int) error {

	return nil
}

func UserLogin(store *Store, username string, password string) error {
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()

	var passwordHash []byte
	err := store.DB.QueryRowContext(
		ctx,
		"SELECT password FROM users WHERE username = ?",
		username,
	).Scan(&passwordHash)
	if err != nil {
		return err
	}

	return bcrypt.CompareHashAndPassword(passwordHash, []byte(password))
}

func InitDB() (*Store, error) {
	db, err := sql.Open("sqlite", "rom-pack.db")
	if err != nil {
		return nil, fmt.Errorf("open database: %w", err)
	}
	db.SetMaxOpenConns(1)
	db.SetMaxIdleConns(1)
	db.SetConnMaxLifetime(0)

	for _, pragma := range []string{
		"PRAGMA foreign_keys = ON",
		"PRAGMA journal_mode = WAL",
		"PRAGMA busy_timeout = 5000",
		"PRAGMA synchronous = NORMAL",
	} {
		if _, err := db.Exec(pragma); err != nil {
			db.Close()
			return nil, fmt.Errorf("configure database: %w", err)
		}
	}
	if err := db.Ping(); err != nil {
		db.Close()
		return nil, fmt.Errorf("ping database: %w", err)
	}
	// construct the rows!
	tx, err := db.Begin()
	if err != nil {
		return nil, err
	}
	defer tx.Rollback()
	for _, statement := range []string{
		`CREATE TABLE IF NOT EXISTS users (
			id INTEGER PRIMARY KEY AUTOINCREMENT,
			username TEXT NOT NULL UNIQUE,
			password TEXT NOT NULL,
			created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
		)`,
	} {
		if _, err := tx.Exec(statement); err != nil {
			return nil, fmt.Errorf("apply schema version 1: %w", err)
		}
	}
	tx.Commit()

	return &Store{DB: db}, nil
}
