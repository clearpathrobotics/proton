from google.protobuf.internal import containers as _containers
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from typing import ClassVar as _ClassVar, Iterable as _Iterable, Mapping as _Mapping, Optional as _Optional, Union as _Union

DESCRIPTOR: _descriptor.FileDescriptor

class ListBools(_message.Message):
    __slots__ = ["bools"]
    BOOLS_FIELD_NUMBER: _ClassVar[int]
    bools: _containers.RepeatedScalarFieldContainer[bool]
    def __init__(self, bools: _Optional[_Iterable[bool]] = ...) -> None: ...

class ListDoubles(_message.Message):
    __slots__ = ["doubles"]
    DOUBLES_FIELD_NUMBER: _ClassVar[int]
    doubles: _containers.RepeatedScalarFieldContainer[float]
    def __init__(self, doubles: _Optional[_Iterable[float]] = ...) -> None: ...

class ListFloats(_message.Message):
    __slots__ = ["floats"]
    FLOATS_FIELD_NUMBER: _ClassVar[int]
    floats: _containers.RepeatedScalarFieldContainer[float]
    def __init__(self, floats: _Optional[_Iterable[float]] = ...) -> None: ...

class ListInt32s(_message.Message):
    __slots__ = ["int32s"]
    INT32S_FIELD_NUMBER: _ClassVar[int]
    int32s: _containers.RepeatedScalarFieldContainer[int]
    def __init__(self, int32s: _Optional[_Iterable[int]] = ...) -> None: ...

class ListInt64s(_message.Message):
    __slots__ = ["int64s"]
    INT64S_FIELD_NUMBER: _ClassVar[int]
    int64s: _containers.RepeatedScalarFieldContainer[int]
    def __init__(self, int64s: _Optional[_Iterable[int]] = ...) -> None: ...

class ListStrings(_message.Message):
    __slots__ = ["strings"]
    STRINGS_FIELD_NUMBER: _ClassVar[int]
    strings: _containers.RepeatedScalarFieldContainer[str]
    def __init__(self, strings: _Optional[_Iterable[str]] = ...) -> None: ...

class ListUint32s(_message.Message):
    __slots__ = ["uint32s"]
    UINT32S_FIELD_NUMBER: _ClassVar[int]
    uint32s: _containers.RepeatedScalarFieldContainer[int]
    def __init__(self, uint32s: _Optional[_Iterable[int]] = ...) -> None: ...

class ListUint64s(_message.Message):
    __slots__ = ["uint64s"]
    UINT64S_FIELD_NUMBER: _ClassVar[int]
    uint64s: _containers.RepeatedScalarFieldContainer[int]
    def __init__(self, uint64s: _Optional[_Iterable[int]] = ...) -> None: ...

class Signal(_message.Message):
    __slots__ = ["bool_value", "bytes_value", "double_value", "float_value", "int32_value", "int64_value", "list_bool_value", "list_double_value", "list_float_value", "list_int32_value", "list_int64_value", "list_string_value", "list_uint32_value", "list_uint64_value", "string_value", "uint32_value", "uint64_value"]
    BOOL_VALUE_FIELD_NUMBER: _ClassVar[int]
    BYTES_VALUE_FIELD_NUMBER: _ClassVar[int]
    DOUBLE_VALUE_FIELD_NUMBER: _ClassVar[int]
    FLOAT_VALUE_FIELD_NUMBER: _ClassVar[int]
    INT32_VALUE_FIELD_NUMBER: _ClassVar[int]
    INT64_VALUE_FIELD_NUMBER: _ClassVar[int]
    LIST_BOOL_VALUE_FIELD_NUMBER: _ClassVar[int]
    LIST_DOUBLE_VALUE_FIELD_NUMBER: _ClassVar[int]
    LIST_FLOAT_VALUE_FIELD_NUMBER: _ClassVar[int]
    LIST_INT32_VALUE_FIELD_NUMBER: _ClassVar[int]
    LIST_INT64_VALUE_FIELD_NUMBER: _ClassVar[int]
    LIST_STRING_VALUE_FIELD_NUMBER: _ClassVar[int]
    LIST_UINT32_VALUE_FIELD_NUMBER: _ClassVar[int]
    LIST_UINT64_VALUE_FIELD_NUMBER: _ClassVar[int]
    STRING_VALUE_FIELD_NUMBER: _ClassVar[int]
    UINT32_VALUE_FIELD_NUMBER: _ClassVar[int]
    UINT64_VALUE_FIELD_NUMBER: _ClassVar[int]
    bool_value: bool
    bytes_value: bytes
    double_value: float
    float_value: float
    int32_value: int
    int64_value: int
    list_bool_value: ListBools
    list_double_value: ListDoubles
    list_float_value: ListFloats
    list_int32_value: ListInt32s
    list_int64_value: ListInt64s
    list_string_value: ListStrings
    list_uint32_value: ListUint32s
    list_uint64_value: ListUint64s
    string_value: str
    uint32_value: int
    uint64_value: int
    def __init__(self, double_value: _Optional[float] = ..., float_value: _Optional[float] = ..., int32_value: _Optional[int] = ..., int64_value: _Optional[int] = ..., uint32_value: _Optional[int] = ..., uint64_value: _Optional[int] = ..., bool_value: bool = ..., string_value: _Optional[str] = ..., bytes_value: _Optional[bytes] = ..., list_double_value: _Optional[_Union[ListDoubles, _Mapping]] = ..., list_float_value: _Optional[_Union[ListFloats, _Mapping]] = ..., list_int32_value: _Optional[_Union[ListInt32s, _Mapping]] = ..., list_int64_value: _Optional[_Union[ListInt64s, _Mapping]] = ..., list_uint32_value: _Optional[_Union[ListUint32s, _Mapping]] = ..., list_uint64_value: _Optional[_Union[ListUint64s, _Mapping]] = ..., list_bool_value: _Optional[_Union[ListBools, _Mapping]] = ..., list_string_value: _Optional[_Union[ListStrings, _Mapping]] = ...) -> None: ...
