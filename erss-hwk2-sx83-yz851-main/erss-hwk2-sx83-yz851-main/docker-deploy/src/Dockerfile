# FROM gcc:9

# RUN mkdir /var/log/erss
# add . /var/log/erss/
# WORKDIR /var/log/erss

# ENTRYPOINT ["./run.sh"]

# 使用官方 Python 镜像作为基础镜像
FROM python:3.9

# 安装 Flask
RUN pip install Flask

# 设置工作目录
WORKDIR /app

# 将 Flask 应用复制到工作目录中
COPY flask-app.py /app

# 暴露端口
EXPOSE 5000

# 运行 Flask 应用
CMD ["python", "flask-app.py"]


