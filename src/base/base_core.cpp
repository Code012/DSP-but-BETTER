internal void 
MemoryCopy(void *dst, const void *src, std::size_t size)
{
	std::memmove(dst, src, size);
}
internal void 
MemoryZero(void *dst, std::size_t size)
{
	std::memset(dst, 0, size);
}

template <typename T>
inline void 
MemoryZeroStruct(T& obj)
{
	MemoryZero(&obj, sizeof(T));
}
template <typename T, std::size_t N>
inline void 
MemoryZeroArray(T (&arr)[N])
{
	MemoryZero(arr, sizeof(arr));
}

internal U64 
DefaultAlign(U64 align) { return Max<U64>(8, align); }
