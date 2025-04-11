#! /usr/bin/bash

WAIT=0
# if there is no .vagrant, first run vagrant up
if [ ! -d ".vagrant" ]; then
  echo "No .vagrant directory found. Please run 'vagrant up' first."
  vagrant up
  WAIT=1
fi

PY="./ssh_client.py"

# Test 1
echo "@@@@@@@@@@Test 1@@@@@@@@@@@@"
echo "Test 1: user sys health"
COMMAND="system_health"
USERNAME="user"
echo "user: $USERNAME, command: $COMMAND"
echo -e "$USERNAME\n$COMMAND" | python3 $PY

# Test 1.5
echo "@@@@@@@@@@Test 1.5@@@@@@@@@@@@"
echo "Test 1.5: user invalid command"
COMMAND="ls -ltrh"
USERNAME="user"
echo "user: $USERNAME, command: $COMMAND"
echo -e "$USERNAME\n$COMMAND" | python3 $PY

# Test 2
echo "@@@@@@@@@@Test 2@@@@@@@@@@@@"
echo "Test 2: user upload file"
COMMAND="upload_file ./send_file /home/normal_user/Downloads/send_file"
USERNAME="user"
echo "user: $USERNAME, command: $COMMAND"
echo -e "$USERNAME\n$COMMAND" | python3 $PY

# Test 3
echo "@@@@@@@@@@Test 3@@@@@@@@@@@@"
echo "Test 3: user upload file invalid path"
COMMAND="upload_file ./send_file /home/normal_user/send_file"
USERNAME="user"
echo "user: $USERNAME, command: $COMMAND"
echo -e "$USERNAME\n$COMMAND" | python3 $PY

# Test 4
echo "@@@@@@@@@@Test 4@@@@@@@@@@@@"
echo "Test 4: admin upload file"
COMMAND="upload_file ./backup_script.sh /home/admin_user/backup_script.sh"
USERNAME="admin"
echo "user: $USERNAME, command: $COMMAND"
echo -e "$USERNAME\n$COMMAND" | python3 $PY

# Test 4
echo "@@@@@@@@@@Test 4-2@@@@@@@@@@@@"
echo "Test 4-2: admin upload file"
COMMAND="upload_file ./schedule_backup.sh /home/admin_user/schedule_backup.sh"
USERNAME="admin"
echo "user: $USERNAME, command: $COMMAND"
echo -e "$USERNAME\n$COMMAND" | python3 $PY

# Test 5
echo "@@@@@@@@@@Test 5@@@@@@@@@@@@"
echo "Test 5: user download file"
COMMAND="download_file ./receive_file /home/normal_user/Downloads/send_file"
USERNAME="user"
echo "user: $USERNAME, command: $COMMAND"
echo -e "$USERNAME\n$COMMAND" | python3 $PY

# Test 6
echo "@@@@@@@@@@Test 6@@@@@@@@@@@@"
echo "Test 6: admin chmod file"
COMMAND="chmod +x ~/*.sh"
USERNAME="admin"
echo "user: $USERNAME, command: $COMMAND"
echo -e "$USERNAME\n$COMMAND" | python3 $PY

# Test 7
echo "@@@@@@@@@@Test 7@@@@@@@@@@@@"
echo "Test 7: admin set backup schedule"
COMMAND="sudo ./schedule_backup.sh \"* * * * *\" \"/var/backup\""
USERNAME="admin"
echo "user: $USERNAME, command: $COMMAND"
echo -e "$USERNAME\n$COMMAND" | python3 $PY

# Test 7-2
echo "@@@@@@@@@@Test 7-2@@@@@@@@@@@@"
echo "Test 7-2: admin check backup schedule"
COMMAND="sudo crontab -l"
USERNAME="admin"
echo "user: $USERNAME, command: $COMMAND"
echo -e "$USERNAME\n$COMMAND" | python3 $PY

# Test 7-3
echo "@@@@@@@@@@Test 7-3@@@@@@@@@@@@"
echo "Test 7-3: admin check backup result"
if [ $WAIT -eq 1 ]; then
  echo "Waiting for 30 seconds to let the backup run..."
  sleep 30
fi
COMMAND="ls -ltrh /var/backup"
USERNAME="admin"
echo "user: $USERNAME, command: $COMMAND"
echo -e "$USERNAME\n$COMMAND" | python3 $PY