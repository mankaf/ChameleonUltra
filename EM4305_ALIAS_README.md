# EM4305 Alias Implementation

Add the following after `lf_em_4x05 = lf.subgroup("4x05", "EM4x05/EM4x69 commands")`:

```python
lf_em4305 = lf.subgroup("em4305", "EM4305/EM4x05/EM4x69 commands (alias)")
```

And add:

```python
@lf_em4305.command('read')
class LfEm4305Read(LFEm4x05Read, ReaderRequiredUnit):
    """Alias for lf em 4x05 read - same implementation"""
    pass

@lf_em4305.command('write')
class LfEm4305WriteT55xx(LFEM410xWriteT55xx, ReaderRequiredUnit):
    """Alias for lf em 4x05 write - same implementation (5 or 13 byte ID)"""
    pass

@lf_em4305.command('econfig')
class LfEm4305Econfig(LFEMIdArgsUnit, SlotIndexArgsAndGoUnit):
    """Alias for lf em 4x05 econfig - same implementation"""
    pass
```
