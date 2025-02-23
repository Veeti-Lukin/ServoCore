import serial
from time import sleep

SERIAL_PORT = "COM6"
SERIAL_BAUDRATE = 115200

TARGET_ID = 0x01
OP_CODE = 0x1
PAYLOAD = bytes([0x1, 0x2, 0x3, 0x4, 0x5])
CRC = 0x00


class RequestPacket:
    def __init__(self, target_id, op_code, payload, crc):
        self.target_id = target_id
        self.op_code = op_code
        self.payload = payload
        self.crc = crc
        self.payload_length = len(payload)
    
    def to_bytes(self):
        return bytes([self.target_id, self.op_code, self.payload_length, self.crc]) + self.payload
    
    def print(self):
        print("RAW:", self.to_bytes())
        print(f"ID: {self.target_id}, OP_CODE: {self.op_code}, PAYLOAD_SIZE: {self.payload_length}, CRC: {self.crc}, PAYLOAD: {self.payload}")

class ResponsePacket:
    def __init__(self, response_code, payload, crc):
        self.response_code = response_code
        self.payload = payload
        self.crc = crc
        self.payload_length = len(payload)
    
    def __init__(self, bytes):
        self.response_code = bytes[0]
        self.payload_length = bytes[1]
        self.crc = bytes[2]
        self.payload = bytes[3::]
    
    def to_bytes(self):
        return bytes([self.response_code, self.payload_length, self.crc]) + self.payload
    
    def print(self):
        print("RAW:", self.to_bytes())
        print(f"RESPONSE CODE: {self.response_code}, PAYLOAD_SIZE: {self.payload_length},  CRC: {self.crc}, PAYLOAD: {self.payload}")




def main():
    try:
        with serial.Serial(SERIAL_PORT, SERIAL_BAUDRATE, timeout=10) as ser:
            request = RequestPacket(TARGET_ID, OP_CODE, PAYLOAD, CRC)
            request.print()

            # Send request
            ser.write(request.to_bytes())

            print("\n------------------------\n")

            # Wait for resposne to appear in the seial buffer
            sleep(1) 
            rx_buffer = bytearray()
            expected_packet_size = None
            
            while ser.in_waiting > 0:
                rx_buffer.append(ser.read(1)[0])  # Read byte-by-byte
                if expected_packet_size is None and len(rx_buffer) >= 2:
                    expected_packet_size = rx_buffer[1] + 3 # Example header-based size detection
                if expected_packet_size and len(rx_buffer) == expected_packet_size:
                    response = ResponsePacket(rx_buffer)
                    response.print()
                    

                    rx_buffer.clear()
                    expected_packet_size = None    
            
    except serial.SerialException as e:
        print(f"Serial error: {e}")
    except KeyboardInterrupt:
        print("\nExiting...")

if __name__ == "__main__":
    main()
