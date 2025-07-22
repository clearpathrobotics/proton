import signal_pb2 as _signal_pb2
from google.protobuf.internal import containers as _containers
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from typing import ClassVar as _ClassVar, Iterable as _Iterable, Mapping as _Mapping, Optional as _Optional, Union as _Union

DESCRIPTOR: _descriptor.FileDescriptor

class Proton(_message.Message):
    __slots__ = ["id", "signals"]
    ID_FIELD_NUMBER: _ClassVar[int]
    SIGNALS_FIELD_NUMBER: _ClassVar[int]
    id: int
    signals: _containers.RepeatedCompositeFieldContainer[_signal_pb2.Signal]
    def __init__(self, id: _Optional[int] = ..., signals: _Optional[_Iterable[_Union[_signal_pb2.Signal, _Mapping]]] = ...) -> None: ...
