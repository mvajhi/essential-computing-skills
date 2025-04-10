#!/bin/bash

# Configuration
OUTPUT_DIR="${OUTPUT_DIR:-/var/backup}"  # Default directory to save backups
BACKUP_DIRS="${BACKUP_DIRS:-/etc /home}" # Default directories to backup
DATE=$(date +%Y%m%d_%H%M%S)
BACKUP_FILE="$OUTPUT_DIR/backup_$DATE.tar.gz"

# Check if the output directory exists and create it if necessary
if [ ! -d "$OUTPUT_DIR" ]; then
  mkdir -p "$OUTPUT_DIR"
  echo "Output directory created: $OUTPUT_DIR"
fi

# Start the backup process
echo "Starting backup of directories: $BACKUP_DIRS"
tar -czvf "$BACKUP_FILE" $BACKUP_DIRS

# Check if the backup was successful
if [ $? -eq 0 ]; then
  echo "Backup completed successfully and saved to: $BACKUP_FILE"
else
  echo "Error during backup."
fi

echo "End of backup script."
