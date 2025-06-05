echo "--- Starting Docker installation ---"
sed -i 's/http:\/\/us./http:\/\/ir./g' /etc/apt/sources.list
sudo apt-get update -y
sudo apt-get install -y ca-certificates curl gnupg lsb-release bash-completion
cat <<EOT >> ~/.bashrc
if [ -f /etc/bash_completion ]; then
    . /etc/bash_completion
fi
EOT
echo "--- Configuring DNS servers ---"
sudo sh -c 'echo "nameserver 178.22.122.100" > /etc/resolv.conf'
sudo sh -c 'echo "nameserver 185.51.200.2" >> /etc/resolv.conf'
sudo mkdir -p /etc/apt/keyrings
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg
echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu \
  $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
sudo apt-get update -y
sudo apt-get install -y docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin
sudo usermod -aG docker vagrant
echo "--- Docker installation complete ---"
echo "--- Configuring Docker daemon.json ---"
sudo mkdir -p /etc/docker
cat <<EOF | sudo tee /etc/docker/daemon.json > /dev/null
{
  "insecure-registries" : ["https://docker.arvancloud.ir"],
  "registry-mirrors": ["https://docker.arvancloud.ir"],
  "bip": "172.19.0.1/16",
  "default-address-pools": [
  {
    "base": "172.30.0.0/16",
    "size": 24
  }
  ]
}
EOF
echo "--- Starting cleanup for box creation ---"
sudo apt-get clean
sudo rm -rf /var/lib/apt/lists/*
sudo rm -rf /tmp/*
sudo rm -f /var/log/vboxadd-install.log
sudo rm -f /var/log/vboxadd-setup.log
# Clean up swap space (optional, if you have a swapfile)
sudo swapoff -a
# If you are sure you have a swapfile and want to remove it, uncomment the next line:
# sudo rm /swapfile
sudo sync
# Zero out free disk space to shrink the box size
sudo dd if=/dev/zero of=/EMPTY bs=1M || true # '|| true' to prevent error if disk is full
sudo rm -f /EMPTY
sync
echo "--- Cleanup for box creation complete ---"