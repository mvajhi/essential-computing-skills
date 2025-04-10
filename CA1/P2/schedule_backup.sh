#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -lt 2 ]; then
  echo "Usage: $0 <run_interval> <output_directory>"
  echo "  <run_interval>: Cron schedule (e.g., '0 3 * * *' for 3 AM daily)"
  echo "  <output_directory>: Directory to save backups (e.g., '/mnt/backup_server')"
  echo "$#"
  exit 1
fi

RUN_INTERVAL="$1"
OUTPUT_DIR="$2"

# Get the full path to the backup script
BACKUP_SCRIPT_PATH=$(readlink -f backup_script.sh)

# Check if the backup script exists and is executable
if [ ! -f "$BACKUP_SCRIPT_PATH" ]; then
  echo "Error: Backup script '$BACKUP_SCRIPT_PATH' not found."
  exit 1
fi

if [ ! -x "$BACKUP_SCRIPT_PATH" ]; then
  echo "Error: Backup script '$BACKUP_SCRIPT_PATH' is not executable. Use 'chmod +x $BACKUP_SCRIPT_PATH'."
  exit 1
fi

# Construct the cron job command
CRON_COMMAND="$RUN_INTERVAL OUTPUT_DIR=\"$OUTPUT_DIR\" $BACKUP_SCRIPT_PATH"

# Remove any existing cron job related to the backup script and add the new one
crontab -l | grep -v "$BACKUP_SCRIPT_PATH" | crontab -
echo "$CRON_COMMAND" | crontab -

echo "Backup of /etc and /home scheduled to run with interval '$RUN_INTERVAL' and output directory '$OUTPUT_DIR'."
echo "Any previous backup schedule has been replaced."
echo "You can check your cron jobs with 'crontab -l'."
