"""Configuration module for Chameleon Ultra."""

import os
from dataclasses import dataclass, field
from typing import Optional


@dataclass
class HardwareConfig:
    """Hardware interface configuration."""
    
    device_id: str = ""
    serial_number: str = ""
    mac_address: str = ""
    hardware_uuid: str = ""
    chip_fingerprint: str = ""


@dataclass
class NetworkConfig:
    """Network configuration for attacks."""
    
    target_interface: str = "eth0"
    relay_port: int = 8443
    listen_port: int = 8080
    max_relay_duration: float = 3600.0  # seconds
    
    @property
    def proxy_url(self) -> str:
        """Get proxy URL for relay attacks."""
        return f"http://localhost:{self.listen_port}"


@dataclass
class ChameleonConfig:
    """Main configuration class."""
    
    hardware: HardwareConfig = field(default_factory=HardwareConfig)
    network: NetworkConfig = field(default_factory=NetworkConfig)
    
    # Feature flags
    enable_relay_attacks: bool = True
    enable_decryption_tools: bool = False  # NOT IMPLEMENTED
    enable_fingerprint_spoofing: bool = False  # NOT IMPLEMENTED
    enable_side_channel: bool = False  # NOT IMPLEMENTED
    enable_hardware_exploits: bool = False  # NOT IMPLEMENTED
    
    @classmethod
    def from_env(cls) -> 'ChameleonConfig':
        """Load configuration from environment variables."""
        config = cls()
        
        # Hardware
        if hw_id := os.getenv('CHAMELEON_HW_ID'):
            config.hardware.device_id = hw_id
        
        # Network
        if target := os.getenv('TARGET_INTERFACE'):
            config.network.target_interface = target
        
        return config


def get_config() -> ChameleonConfig:
    """Get current configuration."""
    return Config.get_config()


class Config:
    """Singleton configuration manager."""
    
    _instance: Optional[ChameleonConfig] = None
    
    @classmethod
    def get_config(cls) -> ChameleonConfig:
        """Get or create configuration instance."""
        if cls._instance is None:
            cls._instance = ChameleonConfig.from_env()
        return cls._instance
    
    @classmethod
    def reset(cls):
        """Reset configuration to defaults."""
        cls._instance = None
