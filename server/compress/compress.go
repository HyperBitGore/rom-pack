package compress

import (
	"io"
	"os"
)

func Compress(tempFilePath string, _ string) (string, string, error) {
	input, err := os.Open(tempFilePath)
	if err != nil {
		return "", "", err
	}
	defer input.Close()

	output, err := os.CreateTemp("", "rom-pack-raw-*")
	if err != nil {
		return "", "", err
	}
	outputPath := output.Name()
	cleanup := true
	defer func() {
		_ = output.Close()
		if cleanup {
			_ = os.Remove(outputPath)
		}
	}()

	if _, err := io.Copy(output, input); err != nil {
		return "", "", err
	}
	if err := output.Sync(); err != nil {
		return "", "", err
	}

	cleanup = false
	return outputPath, "raw", nil
}
