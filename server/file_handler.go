package main

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