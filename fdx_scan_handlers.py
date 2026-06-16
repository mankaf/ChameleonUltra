"""FDX-B scan handler - similar to existing PAC, Viking handlers."""


def fdxb_decode_data(data: bytes) -> tuple[int, int]:
    """
    Decode FDX-B tag data.
    
    FDX-B uses 128-bit UID stored as little-endian integer.
    Format: [2 bytes type + 13 bytes UID]
    
    :param data: Raw response data
    :return: (tag_type, uid)
    """
    if len(data) < 15:
        return -1, 0
    
    tag_type = struct.unpack('!H', data[:2])[0]
    uid = struct.unpack('!13s', data[2:15])[0].hex()
    return tag_type, uid


def paradox_decode_data(data: bytes) -> tuple[int, str]:
    """
    Decode Paradox FSK tag data.
    
    Format: [2 bytes type + 6 bytes UID]
    
    :param data: Raw response data
    :return: (tag_type, uid_hex)
    """
    if len(data) < 8:
        return -1, ""
    
    tag_type = struct.unpack('!H', data[:2])[0]
    uid = struct.unpack('!6s', data[2:8])[0].hex()
    return tag_type, uid


def indala_decode_data(data: bytes) -> int:
    """
    Decode Indala PSK1 tag data.
    
    Format: [6 bytes UID]
    
    :param data: Raw response data
    :return: UID as hex string
    """
    if len(data) < 6:
        return 0
    
    uid = struct.unpack('!6s', data[:6])[0].hex()
    return int.from_bytes(struct.pack('!6s', uid), 'big')


def keri_decode_data(data: bytes) -> int:
    """
    Decode Keri PSK1 tag data.
    
    Format: [6 bytes UID]
    
    :param data: Raw response data
    :return: UID as hex string
    """
    if len(data) < 6:
        return 0
    
    uid = struct.unpack('!6s', data[:6])[0].hex()
    return int.from_bytes(struct.pack('!6s', uid), 'big')
