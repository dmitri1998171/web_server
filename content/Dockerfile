FROM ubuntu
RUN apt update
RUN apt install g++ gcc git make python3 pip libspdlog-dev -y
RUN pip3 install psutil

EXPOSE 18324

COPY ./run_web_app.sh /home
RUN chmod +x /home/run_web_app.sh
ENTRYPOINT ["/home/run_web_app.sh"]