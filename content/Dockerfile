FROM ubuntu
RUN apt update
RUN apt install g++ gcc git make python3 pip libspdlog-dev -y
RUN pip3 install psutil

EXPOSE 18324
ENTRYPOINT ["/home/run_scripts/run_web_app.sh"]