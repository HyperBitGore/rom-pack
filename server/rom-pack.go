package main

import (
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"time"
)

// TODO
//	- db class
//	- login
//	- game uploads
//	- compression


type errorResponse struct {
	Error string `json:"error"`
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

func main () {
	fmt.Println("Starting ROM-Pack!")
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
		Addr: "127.0.0.1:8080",
		Handler: handler,
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