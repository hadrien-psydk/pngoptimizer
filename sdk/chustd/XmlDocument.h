///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_XMLDOCUMENT_H
#define CHUSTD_XMLDOCUMENT_H

#include "Array.h"
#include "String.h"
#include "PtrArray.h"

namespace chustd {

////////////////////////////////////////////////////
// Changes here means changes in TypeToString aswell
enum XmlNodeType
{
	xntUndefined = 0,
	xntElement,
	xntAttribute,        // Currently not used
	xntText,
	xntCDATASection,
	xntEntityReference,  // Currently not used
	xntEntity,           // Currently not used
	xntProcessingInstruction, // Currently not used
	xntComment,
	xntDocument,
	xntDocumentType,     // Currently not used	
	xntDocumentFragment, // Currently not used	
	xntNotation          // Currently not used	
};

/////////////////////////
class XmlNode
{
public:
	static const String& TypeToString(XmlNodeType eType);
public:
	XmlNodeType GetType() const { return m_eType; }		
	virtual ~XmlNode() {}

protected:
	XmlNode(XmlNodeType eType) : m_eType(eType) {}
	
protected:
	XmlNodeType m_eType;
};

/////////////////////////
class XmlAttribute
{
public:
	String m_name;
	String m_strValue;

	XmlAttribute() {}
	XmlAttribute(const XmlAttribute& att) : m_name(att.m_name), m_strValue(att.m_strValue) {}
	XmlAttribute(const String& strName, const String& strValue)
		: m_name(strName), m_strValue(strValue) {}

	XmlAttribute& operator = (const XmlAttribute& att)
	{
		m_name = att.m_name;
		m_strValue = att.m_strValue;
		return *this;
	}
};

////////////////////////////////////////////////////
class XmlElement;
typedef Array<XmlElement*> XmlElementPtrArray;
typedef Array<const XmlElement*> ConstXmlElementPtrArray;

////////////////////////////////////////////////////
class XmlElement : public XmlNode
{
public:
	String m_name;
	Array<XmlAttribute> m_aAttributes;
	
	PtrArray<XmlNode> m_apChildren;
	bool m_bSingleTagElement; // true if <blabla />  false if <blabla></blabla>

public:
	chustd::String	GetAttribute(const String& strName) const;

	XmlNode*       GetFirstChild();
	XmlElement*	   GetFirstElement();
	chustd::String GetFirstText();

	XmlElementPtrArray      GetChildElements(const String& strName);
	ConstXmlElementPtrArray GetChildElements(const String& strName) const;

	XmlElementPtrArray      GetElementsByTagName(const String& strName);

public:
	XmlElement(const String& strName) 
		: XmlNode(xntElement),
		m_name(strName), m_bSingleTagElement(false) {}

	XmlElement(const String& strName, bool bSingle) 
		: XmlNode(xntElement),
		m_name(strName), m_bSingleTagElement(bSingle) {}

	virtual ~XmlElement() {}
};

/////////////////////////
class XmlText : public XmlNode
{
public:
	String m_strData;
public:
	XmlText(const String& str) : XmlNode(xntText), m_strData(str) {}
};

/////////////////////////
class XmlComment : public XmlNode
{
public:
	String m_strData;
public:
	XmlComment(const String& str) : XmlNode(xntComment), m_strData(str) {}
};

class XmlCDATASection : public XmlNode
{
public:
	String m_strData;
public:
	XmlCDATASection(const String& str) : XmlNode(xntCDATASection), m_strData(str) {}
};

////////////////////////////////////////////////////
class XmlDocument
{
public:

	////////////////////////////////////////////////////
	// Possible encoding detected when reading the beginning of the file
	enum DetectedEncoding
	{
		encodingUnknown,

		encodingUtf8,
		encoding8BitsOrUtf8,
		encodingUcs2LeOrUtf16Le,
		encodingUcs2BeOrUtf16Be,
		encodingUcs4LeOrUtf32Le,
		encodingUcs4BeOrUtf32Be
	};

	String DetectedEncodingToString(DetectedEncoding eEncoding);
	
	class XmlHeader
	{
	public:
		Array<XmlAttribute> m_aAttributes;

		XmlHeader()
		{
		}
		
		void Empty()
		{
			m_aAttributes.SetSize(0);
		}
	};
	////////////////////////////////////////////////////

	XmlHeader& GetHeader() { return m_xmlHeader; }

	XmlElement* GetDocumentNode() { return &m_docnode; }
	XmlElement* GetDocumentElement() { return m_docnode.GetFirstElement(); }
	XmlElementPtrArray GetElementsByTagName(const String& strName) { return m_docnode.GetElementsByTagName(strName); }
	XmlElement* GetElementById(const String& strId);

	bool UpdateIdMap();
	bool UpdateIdMap(XmlElement* pStartingElement);
	
	bool Load(const String& filePath);
	bool LoadFromContent(const ByteArray& strContent);
	
	String GetLastErrorString() const;

	void RemoveAll();

	static bool IsBlankChar(int codePoint);
	static bool IsChar(int codePoint);
	static bool IsValidChar(int codePoint);

	XmlDocument();
	virtual ~XmlDocument();

	////////////////////////////////////////////////////////////////////////////
protected:
	bool AddIdInformation(const String& strId, XmlElement* pElement);

protected:
	// This contains a preview of the character encoding, just enough to parse the xml header
	// The final document encoding is a TextEncoding instance found in m_charGiver;
	DetectedEncoding m_eDetectedEncoding;
	bool m_bByteOrderMarkFound;

	XmlHeader m_xmlHeader;
	String m_strDocType;
	XmlElement m_docnode;

	struct IdMapping
	{
		String m_strId;
		XmlElement* m_pNodeElement;
	};
	Array<IdMapping> m_aIds;

	/////////////////////////////////////////////////////////////////

	class CharGiver
	{
	public:
		class State
		{
			friend class CharGiver;
		private:
			int32 m_nNextByteIndex;
			int32 m_nLine;
		
			State() : m_nNextByteIndex(0), m_nLine(0) {}
			void Empty()
			{
				m_nNextByteIndex = 0;
				m_nLine = 1;
			}
		};

		CharGiver()
		{
			m_pTextEncoding = nullptr;
			m_pRawContent = nullptr;
		}

		void Empty()
		{
			m_state.Empty();

			m_pTextEncoding = nullptr;
			m_pRawContent = nullptr;
		}

		void SetContent(const ByteArray* pContent);
		void SetEncoding(const TextEncoding* pTextEncoding)
		{
			m_pTextEncoding = pTextEncoding;
		}
	
		enum ExtractStatus
		{
			esNoError,
			esUnexpectedEndOfFile,
			esBadlyEncodedChar,  // Usually for UTF-8 or UTF-16
			esInvalidCodePoint  // Not a char, such as < 0x1F except 9, A & D, and lonely surrogates and > 0x10ffff
		};

		// Same as private ExtractChar except that it manages end of lines
		ExtractStatus GetNextChar(int& codePoint);

		const CharGiver::State& GetState() const { return m_state; }
		void SetState(const State& state) { m_state = state; }

		int32 GetCurrentLine() const { return m_state.m_nLine; }
		bool IsEndOfBufferReached() const;

	private:
		State m_state;

		const ByteArray* m_pRawContent;
		const TextEncoding* m_pTextEncoding;
	
	private:
		ExtractStatus ExtractChar(int& codePoint, uint32& charLength);
	};
	/////////////////////////////////////////////////////////////////
	CharGiver m_charGiver;
	/////////////////////////////////////////////////////////////////

	String m_strLastError;

protected:
	bool DetectEncoding(const ByteArray& strContent);
	const TextEncoding* GetTemporaryEncoding(); // Gets a text encoding object from the detected encoding
	const TextEncoding* GetFinalEncoding(); // Gets a text encoding object from the detected and declared encoding

	// Returns 0 if error, m_strLastError is filled with the error reason
	uint32 GetNextChar();
	uint32 GetNextNonBlankChar();
	bool IsEndOfBufferReached() const { return m_charGiver.IsEndOfBufferReached(); }

	bool FindImmediate(uint32& cFirst, const String& strWhat);

	bool ParseHeader();
	bool ParseDocType();
	bool ParseComment(String& strComment);
	bool ParseCDataSection(String& strData);

	bool ParseDocument(XmlElement* pParentNode);
	bool ParseElementAttributes(XmlElement* pNode);

	bool ParseAttribute(uint32 cFirst, XmlAttribute& att);
	bool ParseName(uint32 cFirst, String& strName);
	bool ParseReference(uint32 cFirst, String& strResult);
};

} // namespace chustd

#endif // ndef CHUSTD_XMLDOCUMENT_H
