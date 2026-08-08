# Serialization format for server
Base: |4 bytes: len after me|1 bytes: Action (GET, SET)|

# Serialization format for client

Base: |4 bytes: len of everything after me|1 bytes: Type of serialized data:(Array, String)|
if Array then: |number of elements|n str len| str|

if String then:|4 bytes: len of string|n bytes: string|
