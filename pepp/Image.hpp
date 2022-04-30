#pragma once

namespace pepp
{
	enum class PEMachine
	{
		MACHINE_I386 = 0x14c,
		MACHINE_IA64 = 0x200,
		MACHINE_AMD64 = 0x8664
	};

	enum PEDirectoryEntry
	{
		DIRECTORY_ENTRY_EXPORT = 0,   // Export Directory
		DIRECTORY_ENTRY_IMPORT = 1,   // Import Directory
		DIRECTORY_ENTRY_RESOURCE = 2,   // Resource Directory
		DIRECTORY_ENTRY_EXCEPTION = 3,   // Exception Directory
		DIRECTORY_ENTRY_SECURITY = 4,   // Security Directory
		DIRECTORY_ENTRY_BASERELOC = 5,   // Base Relocation Table
		DIRECTORY_ENTRY_DEBUG = 6,   // Debug Directory
		DIRECTORY_ENTRY_ARCHITECTURE = 7,   // Architecture Specific Data
		DIRECTORY_ENTRY_GLOBALPTR = 8,   // RVA of GP
		DIRECTORY_ENTRY_TLS = 9,   // TLS Directory
		DIRECTORY_ENTRY_LOAD_CONFIG = 10,   // Load Configuration Directory
		DIRECTORY_ENTRY_BOUND_IMPORT = 11,   // Bound Import Directory in headers
		DIRECTORY_ENTRY_IAT = 12,   // Import Address Table
		DIRECTORY_ENTRY_DELAY_IMPORT = 13,   // Delay Load Import Descriptors
		DIRECTORY_ENTRY_COM_DESCRIPTOR = 14    // COM Runtime descriptor
	};

	enum class PEMagic
	{
		HDR_32 = 0x10b,
		HDR_64 = 0x20b,
		HDR_ROM = 0x107
	};

	template<unsigned int>
	class PEHeader;
	class SectionHeader;
	template<unsigned int>
	class ExportDirectory;
	template<unsigned int>
	class ImportDirectory;
	template<unsigned int>
	class RelocationDirectory;

	namespace detail
	{
		template<unsigned int bitsize = 0>
		struct Image_t
		{
			using MZHeader_t = IMAGE_DOS_HEADER;
			using ImportDescriptor_t = IMAGE_IMPORT_DESCRIPTOR;
			using BoundImportDescriptor_t = IMAGE_BOUND_IMPORT_DESCRIPTOR;
			using ResourceDirectory_t = IMAGE_RESOURCE_DIRECTORY;
			using ResourceDirectoryEntry_t = IMAGE_RESOURCE_DIRECTORY_ENTRY;
			using SectionHeader_t = IMAGE_SECTION_HEADER;
			using FileHeader_t = IMAGE_FILE_HEADER;
			using DataDirectory_t = IMAGE_DATA_DIRECTORY;
			using ExportDirectory_t = IMAGE_EXPORT_DIRECTORY;
			using RelocationBase_t = IMAGE_BASE_RELOCATION;
			using ImportAddressTable_t = std::uint32_t;
		};

		template<> struct Image_t<64>
		{
			using Header_t = IMAGE_NT_HEADERS64;
			using TLSDirectory_t = IMAGE_TLS_DIRECTORY64;
			using ThunkData_t = IMAGE_THUNK_DATA64;
			using Address_t = std::uint64_t;
			using OptionalHeader_t = IMAGE_OPTIONAL_HEADER64;
		};

		template<> struct Image_t<32>
		{
			using Header_t = IMAGE_NT_HEADERS32;
			using TLSDirectory_t = IMAGE_TLS_DIRECTORY32;
			using ThunkData_t = IMAGE_THUNK_DATA32;
			using Address_t = std::uint32_t;
			using OptionalHeader_t = IMAGE_OPTIONAL_HEADER32;
		};
	}

	/// 
	// - class Image
	// - Used for runtime or static analysis/manipulating of PE files.
	/// 
	template<unsigned int bitsize = 32>
	class Image : pepp::msc::NonCopyable
	{
		using CPEHeader = const PEHeader<bitsize>;
		using CExportDirectory = const ExportDirectory<bitsize>;
		using CImportDirectory = const ImportDirectory<bitsize>;

	public:

		// - Publicize the detail::Image_t used by this image.
		using ImageData_t = detail::Image_t<bitsize>;

		friend class PEHeader<bitsize>;

		static_assert(bitsize == 32 || bitsize == 64, "Invalid bitsize fed into PE::Image");
	private:	
		detail::Image_t<>::MZHeader_t*			m_MZHeader;
		std::string								m_fileName{};
		mem::ByteVector							m_imageBuffer{};
		PEHeader<bitsize>						m_PEHeader;
		// - Sections
		SectionHeader*							m_rawSectionHeaders;
		// - Exports
		ExportDirectory<bitsize>				m_exportDirectory;
		// - Imports
		ImportDirectory<bitsize>				m_importDirectory;
		// - Relocations
		RelocationDirectory<bitsize>			m_relocDirectory;
		// - Is image mapped? Rva2Offset becomes obsolete
		bool									m_isMemMapped = false;
		// - Is image successfully parsed?
		bool									m_isParsed;
	public:

		// - Default ctor.
		Image();

		// - Used to construct a `class Image` via a existing file
		Image(std::string_view filepath);
		
		// - Used to construct a `class Image` via a memory buffer
		Image(const void* data, std::size_t size);

		// - Used to construct via another `class Image`
		Image(const Image& image);

		// - Initialization routines
		void setFromMemory(const void* data, std::size_t size);
		bool setFromMappedMemory(void* data, std::size_t size) noexcept;
		bool setFromFilePath(std::string_view file_path);

		// - Get the start pointer of the buffer.
		std::uint8_t* base() {
			return m_imageBuffer.data();
		}

		mem::ByteVector& buffer() {
			return m_imageBuffer;
		}

		const mem::ByteVector& buffer() const {
			return m_imageBuffer;
		}

		// - Magic number in the DOS header.
		std::uint16_t magic() const {
			return m_MZHeader->e_magic;
		}

		// - Get the Image Base
		Address<> getImageBase() const noexcept {
			return m_PEHeader.getOptionalHdr().getImageBase();
		}

		// - PEHeader wrapper
		class PEHeader<bitsize>& getPEHdr() {
			return m_PEHeader;
		}

		class ExportDirectory<bitsize>& getExportDir() {
			return m_exportDirectory;
		}

		class ImportDirectory<bitsize>& getImportDir() {
			return m_importDirectory;
		}

		class RelocationDirectory<bitsize>& getRelocDir() {
			return m_relocDirectory;
		}

		const PEHeader<bitsize>& getPEHdr() const {
			return m_PEHeader;
		}

		const class ExportDirectory<bitsize>& getExportDir() const {
			return m_exportDirectory;
		}

		const class ImportDirectory<bitsize>& getImportDir() const {
			return m_importDirectory;
		}

		const class RelocationDirectory<bitsize>& getRelocDir() const {
			return m_relocDirectory;
		}

		// - Native pointer
		detail::Image_t<>::MZHeader_t* native() {
			return m_MZHeader;
		}

		// - Assign all sections to the appropriate VA
		void setAsMapped() noexcept;

		void mapToBuffer(pepp::Address<> base, const std::vector<std::string>& ignore = {});

		// - Get PEMachine
		constexpr PEMachine getMachine() const;

		// - Is X64
		static constexpr unsigned int getBitSize() { return bitsize; }

		// - Add a new section to the image
		bool appendSection(std::string_view sectionName, std::uint32_t size, std::uint32_t chars, SectionHeader* out = nullptr);

		// - Extend an existing section (will break things depending on the section)
		bool extendSection(std::string_view sectionName, std::uint32_t delta);

		// - Append a new export
		bool appendExport(std::string_view exportName, std::uint32_t rva);

		void scrambleVaData(uint32_t va, uint32_t size);

		bool isDll() const;
		bool isSystemFile() const;
		bool isDllOrSystemFile() const;

		// - 
		void relocateImage(uintptr_t imageBase);

		// - Find offset padding of value v with count n, starting at specified header or bottom of image if none specified
		std::uint32_t findPadding(SectionHeader* s, std::uint8_t v, std::size_t n, std::uint32_t alignment = 0);

		// - Find offset zero padding up to N bytes, starting at specified header or bottom of image if none specified
		std::uint32_t findZeroPadding(SectionHeader* s, std::size_t n, std::uint32_t alignment = 0);

		// - Find (wildcard acceptable) binary sequence
		std::vector<std::uint32_t> findBinarySequence(SectionHeader* s, std::string_view binary_seq) const;
		std::vector<std::pair<std::int32_t, std::uint32_t>> findBinarySequences(SectionHeader* s, std::initializer_list<std::pair<std::int32_t, std::string_view>> binary_seq) const;

		// - Check if a data directory is "present"
		// - - Necessary before actually using the directory
		// -  (e.g not all images will have a valid IMAGE_EXPORT_DIRECTORY)
		bool hasDataDirectory( PEDirectoryEntry entry);

		// - Write out to file
		void writeToFile(std::string_view filepath);

		// - Wrappers
		SectionHeader& getSectionHdr(std::uint16_t dwIndex) {
			return m_PEHeader.getSectionHeader(dwIndex);
		}
		SectionHeader& getSectionHdr(std::string_view name) {
			return m_PEHeader.getSectionHeader(name);
		}
		SectionHeader& getSectionHdrFromVa(std::uint32_t va) {
			return m_PEHeader.getSectionHeaderFromVa(va);
		} 
		SectionHeader& getSectionHdrFromOffset(std::uint32_t offset) {
			return m_PEHeader.getSectionHeaderFromOffset(offset);
		}
		std::uint16_t getNumberOfSections() const {
			return m_PEHeader.getFileHdr().getNumberOfSections();
		}

		constexpr auto getWordSize() const {
			return bitsize == 64 ? sizeof(std::uint64_t) : sizeof(std::uint32_t);
		}

		bool wasParsed() const {
			return m_isParsed;
		}

	private:
		// - Setup internal objects/pointers and validate they are proper.
		void _validate();
	};

	using Image64 = Image<64>;
	using Image86 = Image<32>;



}