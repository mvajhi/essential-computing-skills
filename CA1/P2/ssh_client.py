import paramiko
import psutil
import os
import inspect  # Import the inspect module

# Define local command handlers
def get_system_health(client, role):
    """
    Retrieves and displays system health information (CPU, memory, disk usage).

    Args:
        client (paramiko.SSHClient): The SSH client object.
        role (str): The role of the user.
    """
    if not check_permission(role, "system_health"):
        print(f"Permission denied: Role '{role}' cannot access system health information.")
        return

    cpu_percent = psutil.cpu_percent()
    memory_percent = psutil.virtual_memory().percent
    disk_percent = psutil.disk_usage('/').percent

    print(f"CPU Usage: {cpu_percent}%")
    print(f"Memory Usage: {memory_percent}%")
    print(f"Disk Usage: {disk_percent}%")

def upload_file(client, role, local_path, remote_path, username):
    """
    Uploads a file to the remote server using SFTP, restricted to /home/USER/Download for 'user' role.

    Args:
        client (paramiko.SSHClient): The SSH client object.
        role (str): The role of the user.
        local_path (str):  The path to the local file.
        remote_path (str): The path to save the file on the server.
        username (str): The username ("admin_user" or "normal_user").
    """

    if not check_permission(role, "upload_file"):
        print(f"Permission denied: Role '{role}' cannot upload files.")
        return

    if role == "user":
        allowed_path = f"/home/{username}/Download"
        if not remote_path.startswith(allowed_path):
            print(f"Error: Uploads for 'user' role are restricted to the {allowed_path} directory.")
            return

    try:
        sftp = client.open_sftp()
        sftp.put(local_path, remote_path)
        sftp.close()
        print(f"File uploaded successfully to {remote_path}")
    except Exception as e:
        print(f"File upload failed: {e}. Please check file permissions and path.")


def download_file(client, role, local_path, remote_path, username):
    """
    Downloads a file from the remote server using SFTP, restricted to /home/USER/Download for 'user' role.

    Args:
        client (paramiko.SSHClient): The SSH client object.
        role (str): The role of the user.
        remote_path (str): The path to the file on the server.
        local_path (str): The path to save the file locally.
        username (str): The username ("admin_user" or "normal_user").
    """
    if not check_permission(role, "download_file"):
        print(f"Permission denied: Role '{role}' cannot download files.")
        return

    # Remove the path restriction for admin
    if role == "user":
        allowed_path = f"/home/{username}/Download"
        if not remote_path.startswith(allowed_path):
            print(f"Error: Downloads for 'user' role are restricted to the {allowed_path} directory.")
            return

    try:
        sftp = client.open_sftp()
        sftp.get(remote_path, local_path)
        sftp.close()
        print(f"File downloaded successfully to {local_path}")
    except Exception as e:
        print(f"File download failed: {e}")

# Define roles and permissions
ROLES = {
    "admin": {
        "can_execute": ["*", "system_health", "upload_file", "download_file"]
    },
    "user": {
        "can_execute": ["system_health", "upload_file", "download_file"]
    }
}

LOCAL_COMMANDS = {
    "system_health": {"func": get_system_health, "remote": False, "sudo": False, "args": ["client", "role"]},
    "upload_file":   {"func": upload_file,   "remote": False, "sudo": False, "args": ["client", "role", "local_path", "remote_path", "username"]},
    "download_file": {"func": download_file, "remote": False, "sudo": False, "args": ["client", "role", "remote_path", "local_path", "username"]},
}

def ssh_connect(host, username, key_path=None):
    """
    Establishes an SSH connection to the specified host.
    Automatically attempts to find an SSH key in common locations if key_path is None.

    Args:
        host (str): The hostname or IP address of the SSH server.
        username (str): The username for authentication.
        key_path (str, optional): Path to the private SSH key file. Defaults to None.

    Returns:
        paramiko.SSHClient: An SSHClient object representing the connection.
        None: If the connection fails.
    """

    client = paramiko.SSHClient()
    client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    try:
        if key_path:
            client.connect(host, username=username, key_filename=key_path)
        else:
            # Attempt to find SSH key in default locations
            default_key_paths = [
                os.path.expanduser("~/.ssh/id_rsa"),
                os.path.expanduser("~/.ssh/id_dsa"),
                os.path.expanduser("~/.ssh/id_ecdsa"),
                os.path.expanduser("~/.ssh/id_ed25519"),
            ]
            for path in default_key_paths:
                if os.path.exists(path):
                    key_path = path
                    break  # Use the first key found
            if key_path:
                client.connect(host, username=username, key_filename=key_path)
            else:
                raise Exception("No SSH key found in default locations. Please specify key_path.")
        print(f"Successfully connected to {host} as {username}")
        return client
    except paramiko.AuthenticationException:
        print("Authentication failed.")
        return None
    except paramiko.SSHException as e:
        print(f"SSH connection error: {e}")
        return None
    except Exception as e:
        print(f"An error occurred: {e}")
        return None

def check_permission(role, command):
    """
    Checks if the given role has permission to execute the command.

    Args:
        role (str): The role of the user.
        command (str): The command to be executed.

    Returns:
        bool: True if the role has permission, False otherwise.
    """

    if role not in ROLES:
        return False
    return "*" in ROLES[role]["can_execute"] or command in ROLES[role]["can_execute"]

def execute_command(client, role, command, *args):
    """
    Executes a command, either locally or remotely, with role-based permission checks.

    Args:
        client (paramiko.SSHClient): The SSH client object.
        role (str): The role of the user executing the command.
        command (str): The command to execute.
        *args: Arguments to pass to the function (local or remote).

    Returns:
        tuple: (stdout, stderr, exit_code) of the executed command, or None if permission is denied or execution fails.
               Returns None for local commands.
    """

    if not check_permission(role, command):
        print(f"Permission denied: Role '{role}' cannot execute command '{command}'")
        return None

    command_details = LOCAL_COMMANDS.get(command)

    if command_details:
        if command_details["func"]:
            # Prepare arguments for the function
            func_args = [client, role]  # Always include client and role
            for arg_name in command_details["args"][2:]:  # Iterate over remaining expected args
                if args:
                    func_args.append(args[0])  # Get the next argument from args
                    args = args[1:]  # Remove the used argument
            command_details["func"](*func_args)
        return None
    else:
        sudo = command_details["sudo"] if command_details else False  # Default to False if not found
        prefix = "sudo " if sudo and role == "admin" else ""
        try:
            # Split the command and its arguments
            command_parts = [prefix + command] + list(args[:-1])
            ssh_command = " ".join(map(str, command_parts))
            stdin, stdout, stderr = client.exec_command(ssh_command)
            stdout_lines = stdout.readlines()
            stderr_lines = stderr.readlines()
            exit_code = stdout.channel.recv_exit_status()
            return "".join(stdout_lines), "".join(stderr_lines), exit_code
        except paramiko.SSHException as e:
            print(f"SSH command execution error: {e}")
            return None
        except Exception as e:
            print(f"An error occurred: {e}")
            return None

if __name__ == "__main__":
    import datetime

    # Server details
    HOST = "192.168.56.12"  # Replace with your server's IP or hostname
    ADMIN_USER = "admin_user"  # Replace with your admin username
    NORMAL_USER = "normal_user"  # Replace with your normal username

    # 1. Choose the role
    while True:
        user_role = input("Enter your role (admin/user): ").lower()
        if user_role in ROLES:
            break
        else:
            print("Invalid role. Please enter 'admin' or 'user'.")

    # Determine the username based on the role
    username = ADMIN_USER if user_role == "admin" else NORMAL_USER

    # 2. Establish SSH connection
    try:
        client = ssh_connect(HOST, username)
        if not client:
            exit()  # Exit if connection fails
    except Exception as e:
        print(f"Error connecting to the server: {e}")
        exit()

    # 3. Get command from user input
    command_parts = input("Enter the command and arguments: ").split()
    command_to_execute = command_parts[0]
    command_args = command_parts[1:]

    # 4. Execute the command and display the output
    print(f"\n--- Executing Command: {command_to_execute} as {user_role} ---")
    result = execute_command(client, user_role, command_to_execute, *command_args, username)

    if result:
        stdout, stderr, exit_code = result
        if stdout:
            print("--- Standard Output ---")
            print(stdout)
        if stderr:
            print("--- Standard Error ---")
            print(stderr)
        print(f"--- Exit Code: {exit_code} ---")
    elif result is None:
        # Local command executed, no output to print from execute_command
        pass
    else:
        print("Command execution failed or was not permitted.")

    # 5. Close the connection
    client.close()