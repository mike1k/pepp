#pragma once

namespace pepp
{
	/*
	* Relocations: research32.blogspot.com/2015/01/base-relocation-table.html
	* Format looks like
	* 00 10 00 00		| RVA of Block
	* 28 01 00 00		| Size of Block
	* ?? ?? ?? ?? .....	| Entries in block
	* (entry count = (reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOC)) / sizeof(WORD))
	*/
	enum RelocationType : std::int8_t
	{
		REL_BASED_ABSOLUTE           =    0,
		REL_BASED_HIGH               =    1,
		REL_BASED_LOW                =    2,
		REL_BASED_HIGHLOW            =    3,
		REL_BASED_HIGHADJ            =    4,
		REL_BASED_MACHINE_SPECIFIC_5 =    5,
		REL_BASED_RESERVED           =    6,
		REL_BASED_MACHINE_SPECIFIC_7 =    7,
		REL_BASED_MACHINE_SPECIFIC_8 =    8,
		REL_BASED_MACHINE_SPECIFIC_9 =    9,
		REL_BASED_DIR64              =    10
	};

	constexpr std::uint16_t craftRelocationBlockEntry(RelocationType type, std::uint16_t offset) noexcept {
		return offset | (type << 12);
	}

	class BlockEntry
	{
		std::uint32_t m_va;
		std::uint16_t m_entry;
	public:
		BlockEntry(std::uint32_t va, std::uint16_t entry)
			: m_va(va)
			, m_entry(entry)
		{
		}

		RelocationType getType() const
		{
			return static_cast<RelocationType>(m_entry >> 12);
		}

		std::uint32_t getOffset() const
		{
			// Single out the last 12 bits of the entry
			return static_cast<std::uint32_t>(m_entry & ((1 << 12) - 1));
		}

		std::uint32_t getRva() const
		{
			return m_va + getOffset();
		}

		constexpr operator std::uint16_t() const
		{
			return m_entry;
		}
	};

	class BlockStream
	{
		std::uint16_t*						 m_base;
		std::uint32_t					     m_idx;
		detail::Image_t<>::RelocationBase_t* m_reloc;
	public:
		BlockStream()
			: m_base(nullptr)
			, m_idx(0)
			, m_reloc(nullptr)
		{
		}

		BlockStream(detail::Image_t<>::RelocationBase_t* reloc)
			: m_base((std::uint16_t*)(reloc + 1))
			, m_idx(0)
			, m_reloc(reloc)
		{
			while (m_base[m_idx])
			{
				++m_idx;
			}
		}

		void append(RelocationType type, std::uint16_t offset)
		{
			if (m_base == nullptr)
				return;

			if (m_idx * sizeof(uint16_t) >= (m_reloc->SizeOfBlock - sizeof(*m_reloc)))
			{
				__debugbreak();
			}

			m_base[m_idx++] = craftRelocationBlockEntry(type, offset);
		}

		std::uint32_t index() const
		{
			return m_idx;
		}

		bool valid() const
		{
			return m_base != nullptr;
		}
	};

	template<unsigned int bitsize>
	class RelocationDirectory : pepp::msc::NonCopyable
	{
		friend class Image<32>;
		friend class Image<64>;

		using PatchType_t = typename detail::Image_t<bitsize>::Address_t;

		Image<bitsize>*							m_image;
		detail::Image_t<>::RelocationBase_t*	m_base;
		SectionHeader*							m_section;
	public:

		int			getNumBlocks() const;
		int			getNumEntries(detail::Image_t<>::RelocationBase_t* reloc) const;
		std::uint32_t	getRemainingFreeBytes() const;
		bool			changeRelocationType(std::uint32_t rva, RelocationType type);
		std::vector<BlockEntry> getBlockEntries(int blockIdx);
		BlockStream createBlock(std::uint32_t rva, std::uint32_t num_entries);
		BlockStream getBlockStream(std::uint32_t rva);
		void extend(std::uint32_t num_entries);
		void forEachEntry(std::function<void(BlockEntry&)> Callback);
		bool isRelocationPresent(std::uint32_t rva) const;
		std::uint32_t getTotalBlockSize();
		void increaseBlockSize(std::uint32_t rva, std::uint32_t num_entries);
		void adjustBlockToFit(uint32_t delta);
		detail::Image_t<>::RelocationBase_t* getBase() { return m_base; }

		bool isPresent() const {
			return m_image->getPEHdr().getOptionalHdr().getDataDir(DIRECTORY_ENTRY_BASERELOC).Size > 0;
		}
	private:
		//! Setup the directory
		void _setup(Image<bitsize>* image) {
			m_image = image;
			m_base = reinterpret_cast<decltype(m_base)>(
				&image->base()[image->getPEHdr().rvaToOffset(
					image->getPEHdr().getOptionalHdr().getDataDir(DIRECTORY_ENTRY_BASERELOC).VirtualAddress)]);
			m_section = 
				&image->getSectionHdrFromVa(image->getPEHdr().getOptionalHdr().getDataDir(DIRECTORY_ENTRY_BASERELOC).VirtualAddress);
		}
	};
}