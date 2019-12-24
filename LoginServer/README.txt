#1 sudo nginx

nginx.conf
{
        location / {
            root   gameServer;
            index  register.html;
        }

        location /reg/{
                include /usr/local/nginx/conf/fastcgi_params;
                fastcgi_pass 127.0.0.1:3600;
        }

        location /login/{
                include /usr/local/nginx/conf/fastcgi_params;
                fastcgi_pass 127.0.0.1:3601;
        }
}

#2 spawn-fcgi -a 127.0.0.1 -p 3600 -f ./reg_cgi
#3 spawn-fcgi -a 127.0.0.1 -p 3601 -f ./login_cgi
#4 redis-server