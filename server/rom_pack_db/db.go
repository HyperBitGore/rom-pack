package rom_pack_db

import (
	"database/sql"
	"fmt"
	_ "modernc.org/sqlite"
)

type Store struct {
	DB *sql.DB
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
			password_hash TEXT NOT NULL,
			created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
		);`,
		`CREATE TABLE IF NOT EXISTS game (
			game_id INTEGER PRIMARY KEY AUTOINCREMENT,
			user_id INTEGER NOT NULL REFERENCES users(id),
			game_name TEXT NOT NULL,
			relative_path TEXT NOT NULL,
			game_size INTEGER NOT NULL,
			checksum TEXT NOT NULL,
			created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
		);`,
		`CREATE TABLE categories (
			id INTEGER PRIMARY KEY AUTOINCREMENT,
			user_id INTEGER NOT NULL
				REFERENCES users(id) ON DELETE CASCADE,
			name TEXT NOT NULL,
			slug TEXT NOT NULL,
			created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
			UNIQUE(user_id, slug)
		);`,
		`CREATE TABLE game_categories (
			game_id INTEGER NOT NULL
				REFERENCES games(id) ON DELETE CASCADE,
			category_id INTEGER NOT NULL
				REFERENCES categories(id) ON DELETE CASCADE,
			added_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
			PRIMARY KEY (game_id, category_id)
		);`,
	} {
		if _, err := tx.Exec(statement); err != nil {
			return nil, fmt.Errorf("apply schema version 1: %w", err)
		}
	}
	tx.Commit()

	return &Store{DB: db}, nil
}
