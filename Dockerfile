FROM ubuntu
COPY . /home
RUN apt update
RUN apt install g++ gcc git make libspdlog-dev