FROM ubuntu
COPY . /home
RUN apt update
RUN apt install g++ gcc git make python3 pip libspdlog-dev -y
RUN pip3 install psutil
CMD cd /home && make && bin/server
# CMD python3 /home/host_metrics_app/main.py docker-compose.yml