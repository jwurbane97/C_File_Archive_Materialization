#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>  // File Proc functions are defined
#include <stdint.h> // Integer types are defined
#include <stdlib.h> // malloc, free are defined
#include <string.h> // strcpy, memset are defined

#pragma pack(push, 1) // 구조체를 1byte 크기로 정렬

// For Archive File
typedef struct _ARCHIVE_HEADER { // Archive Header Structure Definition
	uint16_t magic;    // Archive Magic Number
	uint16_t version;  // Archive Version
} ARCHIVE_HEADER, *PARCHIVE_HEADER;

// For Archive File
typedef struct _FILE_DESC { // File Info Structure Definition
	char name[256];	    // File Name
	uint32_t size;	    // File Size
	uint32_t dataOffset;// File Data Location
} FILE_DESC, *PFILE_DESC;

#pragma pack(pop)

// For program
typedef struct _ARCHIVE {      // Archive Main Structure
	ARCHIVE_HEADER header; // Archive Header
	FILE *fp;	       // Archive File Pointer
} ARCHIVE, *PARCHIVE;

#define ARCHIVE_NAME "archive.bin" // Archive File Name

uint32_t getFileSize(FILE *fp) {
	uint32_t size;
	uint32_t currPos = ftell(fp); // Save the location of current file pointer
	
	// Move to the file's end and get the file size
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);

	fseek(fp, currPos, SEEK_SET); // Remove to last location of file pointer

	return size;
}

// Add files
int append(PARCHIVE archive, char *filename) {
	int ret = 0;
	
	FILE *fp = fopen(filename, "rb");
	if(fp == NULL) {
		printf("%s No file\n", filename);
		return -1;
	}

	uint8_t *buffer;
	uint32_t size;
	
	size = getFileSize(fp);
	buffer = malloc(size);

	if(fread(buffer, size, 1, fp) < 1) {

		printf("Failed to read the file %s\n", filename);
		ret = -1;
		goto Error1; // Move to the place that closes fp and releases the buffer.
	}

	// Create the new file information
	PFILE_DESC desc = malloc(sizeof(FILE_DESC));
	memset(desc, 0, sizeof(FILE_DESC));
	strcpy(desc->name, filename); // Save the filename to be added in the file info structure
	desc->size = size;            // Save the file size to be added in the file info structure
	
	// Move the file pointer next to the archive header
	fseek(archive->fp, sizeof(ARCHIVE_HEADER), SEEK_SET);

	// The start location of the file data is in the location of current file pointer
	// Value : The number of moves (File size)
	desc->dataOffset = ftell(archive->fp) + sizeof(FILE_DESC);

	// Write the new file info in the archive file
	if(fwrite(desc, sizeof(FILE_DESC), 1, archive->fp) < 1) {
		printf("Failed to write the file information\n");
		ret = -1;
		goto Error2; // closes the fp, releases desc and buffer.
	}

	// Write the new file data in the archive file
	if(fwrite(buffer, size, 1, archive->fp) < 1) {
		printf("Failed to write the file information\n");
		ret = -1;
		goto Error2;
	}

	printf("Succeeded to add the file %s, Size: %d\n", filename, size);

Error2:
	free(desc);

Error1: 
	free(buffer);

	fclose(fp);
	
	return ret;

}

int main() {
	PARCHIVE archive = malloc(sizeof(ARCHIVE));
	memset(archive, 0, sizeof(ARCHIVE));

	FILE *fp = fopen(ARCHIVE_NAME, "r+b"); // Open the archive in rw mode

	if(fp == NULL) { // If file isn't exists
		fp = fopen(ARCHIVE_NAME, "w+b");

		if(fp == NULL) return -1;

		archive->header.magic = 'AF'; // Save the magic number AF (FA in little endian)
		archive->header.version = 1; // Save the file version 1
	
		// Save the archive header in the archive file
		if(fwrite(&archive->header, sizeof(ARCHIVE_HEADER), 1, fp) < 1) {
			printf("Failed to write the archive header\n");
			fclose(fp);
			return -1;
		}

		archive->fp = fp; // Save the archive file pointer

		append(archive, "hello.txt"); // Add hello.txt
	}

	fclose(fp); // Close the archive file pointer
	free(archive); // Release the dynamic memory

	return 0;
}
