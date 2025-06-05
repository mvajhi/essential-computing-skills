#!/bin/bash

# Wait until there are 5 lines in 'docker node ls' (1 header + 4 nodes)
while [ "$(docker node ls | wc -l)" -ne 5 ]; do
    echo "Waiting for 4 nodes to join the swarm..." >> /home/vagrant/deploy.log
    sleep 5
done

echo "All 4 nodes have joined. Deploying stack..." >> /home/vagrant/deploy.log
docker stack deploy -c /vagrant/stack.yml mystack >> /home/vagrant/deploy.log 2>&1
echo "Stack deployed successfully." >> /home/vagrant/deploy.log