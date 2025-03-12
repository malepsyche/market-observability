FROM python:3.10-slim
RUN apt-get update && apt-get install -y \
    protobuf-compiler \
    && rm -rf /var/lib/apt/lists/*
RUN pip install --no-cache-dir grpcio grpcio-tools
WORKDIR /app
COPY ./monitoring /app
COPY ./producer/producer_stats.proto /app
RUN python -m grpc_tools.protoc -I/app --python_out=/app --grpc_python_out=/app /app/producer_stats.proto
EXPOSE 50051
CMD ["python", "process_metrics.py"]
