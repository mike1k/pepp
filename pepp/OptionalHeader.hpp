#pragma once

namespace pepp
{
	template<unsigned int bitsize = 32>
	class OptionalHeader : pepp::msc::NonCopyable
	{
		friend class PEHeader<bitsize>;
		friend class Image<bitsize>;

		using ImageData_t = detail::Image_t<bitsize>;

		Image<bitsize>*					m_Image;
		typename ImageData_t::OptionalHeader_t*	m_base{ nullptr };
	public:
		OptionalHeader();


		//! Getter/setter for OptionalHeader.Magic
		void setMagic(PEMagic magic);
		PEMagic getMagic() const;

		//! Getter/setter for OptionalHeader.ImageBase
		void setImageBase(typename detail::Image_t<bitsize>::Address_t address);
		typename detail::Image_t<bitsize>::Address_t getImageBase() const;

		//! Getter/setter for OptionalHeader.SizeOfImage
		void setSizeOfImage(std::uint32_t size);
		std::uint32_t getSizeOfImage() const;

		//! Getter/setter for OptionalHeader.SizeOfCode
		void setSizeOfCode(std::uint32_t dwSize);
		std::uint32_t getSizeOfCode() const;

		//! Getter/setter for OptionalHeader.SizeOfInitializedData
		void setSizeOfInitializedData(std::uint32_t dwSize);
		std::uint32_t getSizeOfInitializedData() const;

		//! Getter/setter for OptionalHeader.SizeOfInitializedData
		void setSizeOfHeaders(std::uint32_t dwSize);
		std::uint32_t getSizeOfHeaders() const;

		//! Getter/setter for OptionalHeader.SizeOfUninitializedData
		void setSizeOfUninitializedData(std::uint32_t dwSize);
		std::uint32_t getSizeOfUninitializedData() const;

		//! Getter/setter for OptionalHeader.BaseOfCode
		void setBaseOfCode(std::uint32_t dwBase);
		std::uint32_t getBaseOfCode() const;

		//! Getter/setter for OptionalHeader.AddressOfEntryPoint
		void setAddressOfEntryPoint(std::uint32_t dwBase);
		std::uint32_t getAddressOfEntryPoint() const;

		//! Getter for OptionalHeader.FileAlignment
		std::uint32_t getFileAlignment() const;

		//! Getter for OptionalHeader.SectionAlignment
		std::uint32_t getSectionAlignment() const;

		//! Get data directory
		detail::Image_t<>::DataDirectory_t& getDataDir(int idx) const {
			return m_base->DataDirectory[idx];
		}

		//! Calculate the number of directories present (not NumberOfRvaAndSizes)
		std::uint32_t getDirectoryCount() const {
			std::uint32_t count{ 0ul };
			for (int i = 0; i < MAX_DIRECTORY_COUNT; i++)
			{
				if (getDataDir(i).Size > 0) {
					++count;
				}
			}
			return count;
		}

		std::uint8_t* base() const {
			return (std::uint8_t*)m_base;
		}

		//! Check if image has relocations
		bool hasRelocations() const;

	private:
		void _setup(Image<bitsize>* image) {
			m_Image = image;
			m_base = &image->getPEHdr().native()->OptionalHeader;
		}
	};
}