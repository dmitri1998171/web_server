FROM ubuntu
RUN apt update
RUN apt install g++ gcc git make python3 pip libspdlog-dev -y
RUN pip3 install psutil

EXPOSE 8080:8080
ENTRYPOINT ["/home/run_scripts/run_server.sh"]