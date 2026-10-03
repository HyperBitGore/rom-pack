package main

import (
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"os"
	"time"

	"rom_pack/rom_pack_db"
)

type config struct {
	AdminUsername string `json:"admin_username"`
	AdminPassword string `json:"admin_password"`
}

type errorResponse struct {
	Error string `json:"error"`
}

func loadConfig(path string) (config, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return config{}, fmt.Errorf("read config: %w", err)
	}

	var cfg config
	if err := json.Unmarshal(data, &cfg); err != nil {
		return config{}, fmt.Errorf("parse config: %w", err)
	}
	if cfg.AdminUsername == "" || cfg.AdminPassword == "" {
		return config{}, fmt.Errorf("admin_username and admin_password are required")
	}

	return cfg, nil
}

func writeJSON(w http.ResponseWriter, status int, value any) {
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(status)
	if value != nil {
		if err := json.NewEncoder(w).Encode(value); err != nil {
			log.Printf("encode response error=%v", err)
		}
	}
}

func writeAPIError(w http.ResponseWriter, status int, message string) {
	writeJSON(w, status, errorResponse{Error: message})
}

func main() {
	fmt.Println("Starting ROM-Pack!")
	store, err := rom_pack_db.InitDB()
	if err != nil {
		log.Fatalf("initialize database: %v", err)
	}
	defer store.DB.Close()

	cfg, err := loadConfig("config.json")
	if err != nil {
		log.Fatalf("load config: %v", err)
	}
	adminExists, err := rom_pack_db.UserExists(store, cfg.AdminUsername)
	if err != nil {
		log.Fatalf("check admin user: %v", err)
	}
	if !adminExists {
		if err := rom_pack_db.AddUser(store, cfg.AdminUsername, cfg.AdminPassword); err != nil {
			log.Fatalf("create admin user: %v", err)
		}
	}

	mux := http.NewServeMux()
	mux.HandleFunc("/alive", func(w http.ResponseWriter, r *http.Request) {
		if r.Method != http.MethodGet {
			writeAPIError(w, http.StatusMethodNotAllowed, "Method not allowed")
			return
		}
		w.WriteHeader(http.StatusOK)
	})
	var handler http.Handler = mux
	server := &http.Server{
		Addr:              "127.0.0.1:8080",
		Handler:           handler,
		ReadHeaderTimeout: 5 * time.Second,
		ReadTimeout:       15 * time.Second,
		WriteTimeout:      30 * time.Second,
		IdleTimeout:       60 * time.Second,
		MaxHeaderBytes:    1 << 20,
	}
	if err := server.ListenAndServe(); err != nil && err != http.ErrServerClosed {
		log.Fatalf("server failed: %v", err)
	}
}
