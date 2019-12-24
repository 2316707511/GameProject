FROM ubuntu
WORKDIR /root/
RUN apt-get update
RUN apt-get install -y redis-tools
COPY game /root/
COPY random_first.txt /root/
COPY random_last.txt /root/
COPY game_start.sh /root/
EXPOSE 6666
ENTRYPOINT ["./game_start.sh"]
