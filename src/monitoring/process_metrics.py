import grpc
import time
from concurrent import futures
import producer_stats_pb2
import producer_stats_pb2_grpc

class ProducerStatsServiceServicer(producer_stats_pb2_grpc.ProducerStatsServiceServicer):
    def SendStats(self, request, context):
        print("\n📊 Received Producer Stats:")
        print(f"➡️ Name: {request.name}")
        print(f"➡️ Client ID: {request.client_id}")
        print(f"➡️ Type: {request.type}")
        print(f"➡️ Age: {request.age}")
        print(f"➡️ Queue Msg Count: {request.msg_cnt}")
        print(f"➡️ TX: {request.tx}")
        print(f"➡️ RX: {request.rx}")
        print(f"➡️ TX Messages: {request.txmsgs}")
        return producer_stats_pb2.Empty()

def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    producer_stats_pb2_grpc.add_ProducerStatsServiceServicer_to_server(ProducerStatsServiceServicer(), server)
    server.add_insecure_port("[::]:50051")
    print("🚀 gRPC server running on port 50051...")
    server.start()
    server.wait_for_termination()

if __name__ == "__main__":
    serve()
