FROM ubuntu
WORKDIR /root/
COPY game /root/
COPY random_first.txt /root/
COPY random_last.txt /root/
COPY game_start.sh /root/
EXPOSE 6666
ENTRYPOINT ["./game_start.sh"]
