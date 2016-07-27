///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XmlDocument.h"
#include "File.h"
#include "StringBuilder.h"
#include "TextEncoding.h"
#include "FormatType.h"
#include "CodePoint.h"

//////////////////////////////////////////////////////////////////////
using namespace chustd;
//////////////////////////////////////////////////////////////////////

const String& XmlNode::TypeToString(XmlNodeType eType)
{
	static String astr[] = 
	{
		"undefined",
		"#element",
		"#attribute",        // Currently not used
		"#text",
		"#cdata-section",
		"#entity-reference",  // Currently not used
		"#entity",           // Currently not used
		"#processing-instruction", // Currently not used
		"#comment",
		"#document",
		"#document-type",     // Currently not used	
		"#document-fragment", // Currently not used	
		"#notation"          // Currently not used	
	};

	const int maxType = (sizeof(astr) / sizeof(String)) - 1;

	int type = int(eType);
	if( type < 0 )
		return astr[maxType];
	
	if( type > maxType )
		return astr[maxType];

	return astr[type];
}
//////////////////////////////////////////////////////////////////////
XmlDocument::XmlDocument() : m_docnode("document")
{
	m_bByteOrderMarkFound = false;
}

XmlDocument::~XmlDocument()
{

}

bool XmlDocument::Load(const String& filePath)
{
	File file;
	if( !file.Open(filePath, chustd::File::modeRead) )
	{
		m_strLastError = "Could not open file";
		return false;
	}

	ByteArray aContent = file.GetContent();
	file.Close();

	return LoadFromContent(aContent);
}

// http://www.w3.org/TR/2004/REC-xml-20040204/#NT-Name
// NameChar ::=	Letter | Digit | '.' | '-' | '_' | ':' | CombiningChar | Extender
// Name	    ::=	(Letter | '_' | ':') (NameChar)*
// Returns true if no erro
bool XmlDocument::ParseName(uint32 cFirst, String& strName)
{
	bool bOkFirst = CodePoint::IsLetter(cFirst) || cFirst == '_' || cFirst == ':';
	if( !bOkFirst )
	{
		m_strLastError = "Bad name";
		return false;
	}

	StringBuilder sb;
	
	sb += cFirst;

	uint32 c;

	while(true)
	{
		CharGiver::State cgsPrevious = m_charGiver.GetState();

		c = GetNextChar();
		if( c == 0 || IsBlankChar(c) )
		{
			// End of the name
			m_charGiver.SetState(cgsPrevious);
			break;
		}
	
		bool bOk = CodePoint::IsLetter(c) || CodePoint::IsDigit(c) || c == '.' || c == '-'
			|| c == '_' || c == ':' || CodePoint::IsCombiningChar(c) || CodePoint::IsExtenderChar(c);

		if( !bOk )
		{
			// End of the name
			m_charGiver.SetState(cgsPrevious);
			break;
		}

		sb += c;
	}

	strName = sb.ToString();

	return true;
}


bool XmlDocument::ParseReference(uint32 cFirst, String& strResult)
{
	// Reference ::= EntityRef | CharRef
	// EntityRef ::= '&' Name ';'
	// CharRef   ::= '&#' [0-9]+ ';' | '&#x' [0-9a-fA-F]+ ';'

	if( cFirst != '&' )
	{
		ASSERT(0);
		return false;
	}

	static const char szErrInvalidChar[] = "Reference to invalid character number '";

	StringBuilder sbReference;
	String strReference;

	uint32 c = GetNextChar();
	if( c == 0 )
	{
		m_strLastError = "Unfinished reference";
		return false;
	}

	if( c == '#' )
	{
		// CharRef

		const int32 maxCharRef = 0x10ffff; // 1114111 in decimal

		c = GetNextChar();
		if( c == 0 )
		{
			m_strLastError = "Unfinished reference";
			return false;
		}

		int32 ucs4Char = 0;
		char cArg = 0;

		if( c == 'x' )
		{
			// Hexadecimal CharRef
			c = GetNextChar();

			while( ('0' <= c && c <= '9')
			    || ('a' <= c && c <= 'f')
			    || ('A' <= c && c <= 'F') )
			{
				sbReference += c;
				c = GetNextChar();
			}

			if( c != ';' )
			{
				// Bad end of reference
				m_strLastError = "; expected at the end of reference '" + sbReference.ToString() + "'";
				return false;
			}

			// 7 = number of characters in 1114111
			if( sbReference.GetLength() > 7 )
			{
				m_strLastError = szErrInvalidChar + sbReference.ToString() + "'";
				return false;
			}

			cArg = 'x';
			strReference = sbReference.ToString();
		}
		else
		{
			// Decimal CharRef
			while( '0' <= c && c <= '9')
			{
				sbReference += c;
				c = GetNextChar();
			}

			strReference = sbReference.ToString();

			if( c != ';' )
			{
				// Bad end of reference
				m_strLastError = "; expected at the end of reference '" + strReference + "'";
				return false;
			}

			// 7 = number of characters in 1114111
			if( strReference.GetLength() > 7 )
			{
				m_strLastError = szErrInvalidChar + strReference + "'";
				return false;
			}
		}
		
		if( strReference.IsEmpty() )
		{
			m_strLastError = "Empty character reference";
			return false;
		}

		strReference.ToInt(ucs4Char, cArg);

		if( ucs4Char > maxCharRef )
		{
			m_strLastError = szErrInvalidChar + strReference + "'";
			return false;
		}

		strResult = ucs4Char;
	}
	else
	{
		// EntityRef
		if( !ParseName(c, strReference) )
		{
			m_strLastError = "Invalid name for entity reference";
			return false;
		}

		c = GetNextChar();
		if( c != ';' )
		{
			m_strLastError = "; expected at the end of reference '" + strReference + "'";
			return false;
		}

		// Entity Reference
		if( strReference == "lt" )
		{
			strResult = '<';
		}
		else if( strReference == "gt" )
		{
			strResult = '>';
		}
		else if( strReference == "amp" )
		{
			strResult = '&';
		}
		else if( strReference == "apos" )
		{
			strResult = '\'';
		}
		else if( strReference == "quot" )
		{
			strResult = '"';
		}
		else
		{
			m_strLastError = "Unknown entity reference '&" + strReference + ";'";
			return false;
		}
	}
	return true;
}

// attname="attvalue"
// \___ a = cFirst

bool XmlDocument::ParseAttribute(uint32 cFirst, XmlAttribute& att)
{
	if( !ParseName(cFirst, att.m_name) )
	{
		m_strLastError = "Error in attribute name : " + m_strLastError;
		return false;
	}

	uint32 c = GetNextNonBlankChar();
	if( c != '=' )
	{
		m_strLastError = "= missing after attribute name '" + att.m_name + "'";
		return false;
	}

	c = GetNextNonBlankChar();
	if( c != '"' &&  c != '\'' )
	{
		m_strLastError = "Opening double quote missing for attribute value '" + att.m_name + "'";
		return false;
	}

	bool bDoubleQuote = (c == '"');

	StringBuilder sbValue;

	while(true)
	{
		c = GetNextChar();
		if( c == 0 )
			break;

		if( bDoubleQuote && c == '"' )
			break;

		if( !bDoubleQuote && c == '\'' )
			break;

		if( c == '<' )
		{
			m_strLastError = "< forbidden inside for attribute value '" + att.m_name + "', use &lt; instead";
			return false;
		}
		if( c == '>' )
		{
			m_strLastError = "> forbidden inside for attribute value '" + att.m_name + "', use &gt; instead";
			return false;
		}

		if( c == '&' )
		{
			String strResult;
			if( !ParseReference(c, strResult) )
				return false;

			sbValue += strResult;
		}
		else
		{		
			sbValue += c;
		}
	}

	att.m_strValue = sbValue.ToString();

	if( bDoubleQuote && c != '"' )
	{
		m_strLastError = "Closing double quote missing for attribute value '" + att.m_name + "'";
		return false;
	}

	if( !bDoubleQuote && c != '\'' )
	{
		m_strLastError = "Closing quote missing for attribute value '" + att.m_name + "'";
		return false;
	}

	return true;
}

// Returns true if the current poped char and the following chars matches the string
// Advance of the string length if the match succeed, otherwise keep the current position
bool XmlDocument::FindImmediate(uint32& cFirst, const String& strWhat)
{
	uint32 c = cFirst;
	const int32 length = strWhat.GetLength();
	if( length == 0 )
		return true;

	CharGiver::State cgsPrevious = m_charGiver.GetState();

	int32 i = 0;
	while(true)
	{
		if( c != strWhat.GetAt(i) )
		{
			// Restore the previous char giver state
			m_charGiver.SetState(cgsPrevious);
			return false;
		}
		++i;

		if( i == length )
			break;

		c = GetNextChar();
	}
	
	return true;
}

bool XmlDocument::ParseHeader()
{
	enum { stateFirstFindLt, stateFirstFindInter, stateXmlString, stateGetHeaderAttributes };
	int state = stateFirstFindLt;

	static const char szErr0[] = "<?xml expected at the beginning of the document";
	static const char szErr1[] = "?> closing the header not found";
		
	String strAttName;
	String strAttValue;

	while(true)
	{
		uint32 c = GetNextChar();
		if( c == 0 )
			break;

		if( state == stateFirstFindLt )
		{
			if( IsBlankChar(c) )
				continue;

			if( c != '<' )
			{
				m_strLastError = szErr0;
				return false;
			}
			state = stateFirstFindInter;
		}
		else if( state == stateFirstFindInter )
		{
			if( c != '?' )
			{
				m_strLastError = szErr0;
				return false;
			}
			state = stateXmlString;
		}
		else if( state == stateXmlString )
		{
			if( !FindImmediate(c, "xml") )
			{
				m_strLastError = szErr0;
				return false;
			}
			
			state = stateGetHeaderAttributes;
		}
		else if( state == stateGetHeaderAttributes )
		{
			if( FindImmediate(c, "?>") )
			{
				return true;
			}
			
			if( !IsBlankChar(c) )
			{
				// Not the end of the header nor a blank char, it should be an attribute name
				XmlAttribute att;
				if( !ParseAttribute(c, att) )
					return false;

				m_xmlHeader.m_aAttributes.Add(att);
			}
		}
	}

	m_strLastError = szErr1;
	return false;
}

bool XmlDocument::DetectEncoding(const ByteArray& aContent)
{
	static const char szErr1[] = "Bad character encoding, strange byte order";
	static const char szErr2[] = "Bad content, no xml header found";

	// Auto-detect encoding
	if( aContent.GetSize() < 4 )
	{
		m_strLastError = "Uncomplete xml file";
		return false;
	}

	uint8 n0 = aContent[0];
	uint8 n1 = aContent[1];
	uint8 n2 = aContent[2];
	uint8 n3 = aContent[3];

	////////////////////////////////////////////////////////////
	// Test with byte order mark

	m_bByteOrderMarkFound = true;

	////////////////////////////////////////////////////////////
	// UCS-4
	if( n0 == 0 && n1 == 0 && n2 == 0xfe && n3 == 0xff )
	{
		m_eDetectedEncoding = encodingUcs4BeOrUtf32Be;
		return true;
	}

	if( n0 == 0xff && n1 == 0xfe && n2 == 0 && n3 == 0 )
	{
		m_eDetectedEncoding = encodingUcs4LeOrUtf32Le;
		return true;
	}

	if( n0 == 0 && n1 == 0 && n2 == 0xff && n3 == 0xfe )
	{
		// Unusual byte order
		m_strLastError = szErr1;
		return false;
	}

	if( n0 == 0xfe && n1 == 0xff && n2 == 0 && n3 == 0 )
	{
		// Unusual byte order
		m_strLastError = szErr1;
		return false;
	}
	////////////////////////////////////////////////////////////

	// UCS-2 / UTF-16
	if( n0 == 0xfe && n1 == 0xff && n2 == 0 && n3 != 0x3c )
	{
		m_eDetectedEncoding = encodingUcs2BeOrUtf16Be;
		return true;
	}

	if( n0 == 0xff && n1 == 0xfe && n2 == 0x3c && n3 == 0 )
	{
		m_eDetectedEncoding = encodingUcs2LeOrUtf16Le;
		return true;
	}

	// UTF-8
	if( n0 == 0xef && n1 == 0xbb && n2 == 0xbf )
	{
		m_eDetectedEncoding = encodingUtf8;
		return true;
	}

	////////////////////////////////////////////////////////////
	// Test without byte order mark

	m_bByteOrderMarkFound = false;

	if( n0 == 0 && n1 == 0 && n2 == 0 && n3 == 0x3c )
	{
		m_eDetectedEncoding = encodingUcs4BeOrUtf32Be;
		return true;
	}

	if( n0 == 0x3c && n1 == 0 && n2 == 0 && n3 == 0 )
	{
		m_eDetectedEncoding = encodingUcs4LeOrUtf32Le;
		return true;
	}

	if( n0 == 0 && n1 == 0 && n2 == 0x3c && n3 == 0 )
	{
		m_strLastError = szErr1;
		return false;
	}

	if( n0 == 0 && n1 == 0x3c && n2 == 0 && n3 == 0 )
	{
		m_strLastError = szErr1;
		return false;
	}

	if( n0 == 0 && n1 == 0x3c && n2 == 0 && n3 == 0x3f )
	{
		m_eDetectedEncoding = encodingUcs2BeOrUtf16Be;
		return true;
	}

	if( n0 == 0x3c && n1 == 0 && n2 == 0x3f && n3 == 0 )
	{
		m_eDetectedEncoding = encodingUcs2LeOrUtf16Le;
		return true;
	}
	
	if( n0 == 0x3c && n1 == 0x3f && n2 == 0x78 && n3 == 0x6D )
	{
		m_eDetectedEncoding = encoding8BitsOrUtf8;
		return true;
	}

	m_strLastError = szErr2;
	return false;
}

void XmlDocument::CharGiver::SetContent(const ByteArray* pContent)
{
	Empty();
	m_pRawContent = pContent;
}

String XmlDocument::DetectedEncodingToString(DetectedEncoding eEncoding)
{
	switch(eEncoding)
	{
	case encodingUnknown:
		return "Unknown";
	case encodingUtf8:
		return "UTF-8";
	// Only with automatic detection
	case encoding8BitsOrUtf8:
		return "either 8 bits encoded characters like ISO-8859-*, or either UTF-8";
	case encodingUcs2BeOrUtf16Be:
		return "either UCS-2 Big Endian or UTF-16 Big Endian";
	case encodingUcs2LeOrUtf16Le:
		return "either UCS-2 Little Endian or UTF-16 Little Endian";
	case encodingUcs4BeOrUtf32Be:
		return "either UCS-4 Big Endian or UTF-32 Big Endian";
	case encodingUcs4LeOrUtf32Le:
		return "either UCS-4 Little Endian or UTF-32 Little Endian";
	default:
		break;
	}

	return "no encoding information";
}

// Gets an instance of TextEncoding depending on the detected encoding
const TextEncoding* XmlDocument::GetTemporaryEncoding()
{
	switch(m_eDetectedEncoding)
	{
	case encodingUnknown:
		return nullptr;

	case encodingUtf8:
		return &TextEncoding::Utf8();
	case encoding8BitsOrUtf8:
		return &TextEncoding::Iso8859_1();
	case encodingUcs2LeOrUtf16Le:
		return &TextEncoding::Utf16Le();
	case encodingUcs2BeOrUtf16Be:
		return &TextEncoding::Utf16Be();
	case encodingUcs4LeOrUtf32Le:
		return &TextEncoding::Utf32Le();
	case encodingUcs4BeOrUtf32Be:
		return &TextEncoding::Utf32Be();

	default:
		return nullptr;
	}
}

// Gets an instance of TextEncoding depending on the detected encoding and the declared encoding
// Example : <?xml version="1.0" encoding="windows-1252"?>
const TextEncoding* XmlDocument::GetFinalEncoding()
{
	const int32 attributeCount = m_xmlHeader.m_aAttributes.GetSize();
	
	String strEncoding;
	for(int i = 0; i < attributeCount; ++i)
	{
		if( m_xmlHeader.m_aAttributes[i].m_name == "encoding" )
		{
			strEncoding = m_xmlHeader.m_aAttributes[i].m_strValue;
			break;
		}
	}

	if( strEncoding.IsEmpty() )
	{
		// Empty, then use the default encoding
		return &TextEncoding::Utf8();
	}

	// Find the declared encoding in our known encodings
	strEncoding = strEncoding.ToUpperCase();
	
	const Array<const TextEncoding*>& apKnownEncodings = TextEncoding::GetKnownEncodings();
	const int count = apKnownEncodings.GetSize();
	for(int i = 0; i < count; ++i)
	{
		const TextEncoding* pTE = apKnownEncodings[i];
		String strName = pTE->GetName();
		strName = strName.ToUpperCase();

		if( strName == strEncoding )
		{
			return pTE; // Found ! ^^
		}
	}

	// Mmh, ok, the encoding was not found, but maybe the declared name is compatible with
	// one of those we know

	if( strEncoding == "UTF-16" || strEncoding == "UCS-2" )
	{
		if( m_eDetectedEncoding == encodingUcs2LeOrUtf16Le )
		{
			return &TextEncoding::Utf16Le();
		}

		if( m_eDetectedEncoding == encodingUcs2BeOrUtf16Be )
		{
			return &TextEncoding::Utf16Be();
		}
	}
	else if( strEncoding == "UTF-32" || strEncoding == "UCS-4" )
	{
		if( m_eDetectedEncoding == encodingUcs4LeOrUtf32Le )
		{
			return &TextEncoding::Utf32Le();
		}

		if( m_eDetectedEncoding == encodingUcs4BeOrUtf32Be )
		{
			return &TextEncoding::Utf32Be();
		}
	}
	else
	{
		// This encoding is not supported
		m_strLastError = "Encoding not supported : " + strEncoding;
		return nullptr;
	}

	m_strLastError = "Inconsistency between document encoding detection and declaration. "
		"The parser found that the document is encoded as "
		+ DetectedEncodingToString(m_eDetectedEncoding)
		+ " but the xml header declares the encoding as "
		+ strEncoding;

	return nullptr;
}

bool XmlDocument::IsBlankChar(int codePoint)
{
	uint16 c = uint16(codePoint);
	return CodePoint::IsWhitespace(c);
}

bool XmlDocument::IsChar(int codePoint)
{
	if( codePoint == 0x09 || codePoint == 0x0A || codePoint == 0x0D )
		return true;
	
	if( 0x20 <= codePoint && codePoint <= 0xD7FF )
		return true;
	
	if( 0xE000 <= codePoint && codePoint <= 0xFFFD )
		return true;
	
	if( 0x10000 <= codePoint && codePoint <= 0x10FFFF )
		return true;

	return false;
}

bool XmlDocument::ParseCDataSection(String& strData)
{
	// CDSect   ::=	CDStart CData CDEnd
	// CDStart	::=	'<![CDATA['
	// CData   ::=	(Char* - (Char* ']]>' Char*))
	// CDEnd   ::=	']]>'

	enum { stateCalm, stateOneSquareBracket, stateTwoSquareBrackets };
	int state = stateCalm;

	StringBuilder sbData;

	while(true)
	{
		uint32 c = GetNextChar();
		if( c == 0 )
		{
			m_strLastError = m_strLastError + " in CDATA section";
			break;
		}

		if( state == stateCalm )
		{
			if( c == ']' )
			{
				state = stateOneSquareBracket;
			}
			else
			{
				sbData += c;
			}
		}
		else if( state == stateOneSquareBracket )
		{
			if( c == ']' )
			{
				state = stateTwoSquareBrackets;
			}
			else
			{
				sbData += c;

				state = stateCalm;
			}
		}
		else if( state == stateTwoSquareBrackets )
		{
			if( c == '>' )
			{
				// End of the CDATA section
				return true;
			}

			// Continue
			sbData += "]]";

			state = stateCalm;
		}
	}

	strData = sbData.ToString();
	return false;
}

bool XmlDocument::ParseComment(String& strComment)
{
	// Comment ::= '<!--' ((Char - '-') | ('-' (Char - '-')))* '-->'

	enum { stateCalm, stateOneHyphen, stateTwoHyphens };
	int state = stateCalm;

	StringBuilder sbComment;

	while(true)
	{
		uint32 c = GetNextChar();
		if( c == 0 )
		{
			m_strLastError = m_strLastError + " in XML comment";
			break;
		}

		if( state == stateCalm )
		{
			if( c == '-' )
			{
				state = stateOneHyphen;
			}
			else
			{
				sbComment += c;
			}
		}
		else if( state == stateOneHyphen )
		{
			if( c == '-' )
			{
				state = stateTwoHyphens;
			}
			else
			{
				sbComment += c;

				state = stateCalm;
			}
		}
		else if( state == stateTwoHyphens )
		{
			if( c == '>' )
			{
				// End of the comment
				break;
			}

			// Malformed comment
			m_strLastError = "-- forbidden in comment";
			return false;
		}
	}

	strComment = sbComment.ToString();
	return true;
}

bool XmlDocument::ParseDocType()
{
	StringBuilder sbDocType;
	
	while(true)
	{
		uint32 c = GetNextChar();
		if( c == 0 )
		{
			m_strLastError = m_strLastError + " in DOCTYPE";
			break;
		}
		if( c == '>' )
		{
			return true;
		}

		sbDocType += c;
	}

	m_strDocType = sbDocType.ToString();
	return false;
}

// Parses the attributes, and sets the m_bSingleTagElement value in the node
bool XmlDocument::ParseElementAttributes(XmlElement* pNode)
{
	while(true)
	{
		uint32 c = GetNextChar();
		if( c == 0 )
		{
			m_strLastError = m_strLastError + " in attributes";
			break;
		}

		if( c == '>' )
		{
			// End of opening tag
			pNode->m_bSingleTagElement = false;
			return true;
		}
		
		static const String strEndTag = "/>";
		if( FindImmediate(c, strEndTag) )
		{
			// End of opening tag, and this is a single tag element
			pNode->m_bSingleTagElement = true;
			return true;
		}
		else if( !IsBlankChar(c) )
		{
			XmlAttribute att;
			if( !ParseAttribute(c, att) )
				return false;

			pNode->m_aAttributes.Add(att);
		}
	}
	return false;
}

bool XmlDocument::ParseDocument(XmlElement* pParentNode)
{
	static const char szErrBadElementName[] = "Bad element name";
	static const char szErrGarbageBeforeDocumentElement[] = "Garbage found before document element";
	/////////////////////////////////////////////////////////////////
	
	// Current TextNode text
	StringBuilder sbText;

	enum { stateCalm, stateLtFound };
	int state = stateCalm;

	while( !IsEndOfBufferReached())
	{
		uint32 c = GetNextChar();
		if( c == 0 )
		{
			return false;
		}

		if( state == stateCalm )
		{
			if( c == '<' )
			{
				state = stateLtFound;
				
				if( !sbText.IsEmpty() )
				{
					// We were gathering text
					XmlText* pText = new XmlText(sbText.ToString());
					pParentNode->m_apChildren.Add(pText);

					sbText.Empty(); // Reset for next text block
				}
			}	
			else
			{
				// Inside text data
				if( pParentNode == &m_docnode )
				{
					// We are in the top level of the document, we cannot add text but we
					// can have blank chars
					if( !IsBlankChar(c) )
					{
						m_strLastError = szErrGarbageBeforeDocumentElement;
						return false;
					}
				}
				else
				{
					if( c == '&' )
					{
						String strResult;
						if( !ParseReference(c, strResult) )
							return false;

						sbText += strResult;
					}
					else
					{
						sbText += c;
					}
				}
				
			}
		}
		else if( state == stateLtFound )
		{
			if( c == '!' )
			{
				// Maybe a comment
				c = GetNextChar();
				if( c == 0 )
				{
					return false;
				}
				
				if( FindImmediate(c, "--") )
				{
					String strComment;
					if( !ParseComment(strComment) )
						return false;

					// End of comment
					XmlComment* pComment = new XmlComment(strComment);
					pParentNode->m_apChildren.Add(pComment);
				}
				else if( FindImmediate(c, "DOCTYPE") )
				{
					if( !ParseDocType() )
						return false;
				}
				else if( FindImmediate(c, "[CDATA[") )
				{
					if( pParentNode == &m_docnode )
					{
						m_strLastError = szErrGarbageBeforeDocumentElement;
						return false;
					}

					String strData;
					if( !ParseCDataSection(strData) )
						return false;

					// End of comment
					XmlCDATASection* pCDATASection = new XmlCDATASection(strData);
					pParentNode->m_apChildren.Add(pCDATASection);
				}
				else
				{
					m_strLastError = szErrBadElementName;
					return false;
				}

				// End of the comment child, we continue in calm mode
				state = stateCalm;
			}
			else if( c == '/' )
			{
				//////////////////////////////////////
				// There is a closing tag here
				
				// Jump over the slash
				c = GetNextChar();
				if( c == 0 )
				{
					return false;
				}

				// Get the closing tag name
				String strClosingTagName;
				if( !ParseName(c, strClosingTagName) )
					return false;
				
				// Get the > or the ' '
				c = GetNextChar();
				if( c == 0 )
				{
					return false;
				}
				if( c == ' ' )
				{
					// Jump over blank
					c = GetNextChar();
				}

				if( c != '>' )
				{
					m_strLastError = "Garbage found in closing tag </" + strClosingTagName + "...";
					return false;
				}

				// End of closing tag and end of the loop

				// We check that the closing tag name matches the opening tag
				XmlElement* pParentElement = (XmlElement*) pParentNode;
				if( strClosingTagName == pParentElement->m_name )
				{
					return true;
				}
				m_strLastError = "Cannot interlace elements. " + String("</") + strClosingTagName 
					+ "> unexpected here.";
				return false;
			}
			else
			{
				//////////////////////////////////////
				// There is an opening tag here
				
				// Note : blank chars between the < and the name are forbidden
				// '<' Name (S Attribute)* S? '>'
				// S = blank char

				// Get the opening tag name
				String strOpeningTagName;
				if( !ParseName(c, strOpeningTagName) )
				{
					m_strLastError = m_strLastError + " for opening tag name";
					return false;
				}

				if( pParentNode == &m_docnode )
				{
					// This is the root of the document !
					if( GetDocumentElement() != nullptr )
					{
						// Error, only one root in an xml document
						m_strLastError = "More than one root in document";
						return false;
					}
				}

				XmlElement* pChild = new XmlElement(strOpeningTagName);
				pParentNode->m_apChildren.Add(pChild);
				

				// Parse attributes
				if( !ParseElementAttributes(pChild) )
				{
					return false;
				}

				if( !pChild->m_bSingleTagElement )
				{
					// Go inside
					if( !ParseDocument(pChild) )
						return false;
				}
				
				// End of the child, we continue in calm mode
				state = stateCalm;
			}
		}
		else
		{
			ASSERT(0); // Undefined state
		}
	}
	return true;
}

bool XmlDocument::LoadFromContent(const ByteArray& aContent)
{
	RemoveAll();

	if( !DetectEncoding(aContent) )
	{
		return false;
	}

	m_docnode.m_apChildren.Clear();
	m_strDocType.Empty();
	m_charGiver.SetContent(&aContent);

	// We may not know the exact encoding, but we know enough to parse the header, and
	// get additional character encoding information.
	const TextEncoding* pTmpTextEncoding = GetTemporaryEncoding();
	ASSERT(pTmpTextEncoding != nullptr);

	m_charGiver.SetEncoding(pTmpTextEncoding);

	// We remove the byte order mark as the header parser won't like it
	if( m_bByteOrderMarkFound )
	{
		GetNextChar();
	}

	if( !ParseHeader() )
	{
		return false;
	}

	// Compare the automatic encoding and the declared encoding to check if they match
	const TextEncoding* pFinalTextEncoding = GetFinalEncoding();
	if( pFinalTextEncoding == nullptr )
	{
		return false;
	}

	// Yes ! Here we are, with the final encoding ! ^^,
	m_charGiver.SetEncoding(pFinalTextEncoding);

	/////////////////////////////////////////////////////////////////
	// Get the real job done now
	if( !ParseDocument(&m_docnode) )
	{
		return false;
	}

	if( !UpdateIdMap() )
	{
		return false;
	}

	return true;
}

// Returns the Unicode code point, or 0 if an error occured
uint32 XmlDocument::GetNextChar()
{
	int codePoint;
	XmlDocument::CharGiver::ExtractStatus eStatus = m_charGiver.GetNextChar(codePoint);
	if( eStatus == CharGiver::esNoError )
		return codePoint;

	switch(eStatus)
	{
	case CharGiver::esUnexpectedEndOfFile:
		m_strLastError = "Unexpected end of file";
		break;
	case CharGiver::esBadlyEncodedChar:
		m_strLastError = "Badly encoded character";
		break;
	case CharGiver::esInvalidCodePoint:
		m_strLastError = "Invalid character";
		break;
	default:
		break;
	}
	return 0;
}

uint32 XmlDocument::GetNextNonBlankChar()
{
	uint32 c = 0;

	while(true)
	{
		c = GetNextChar();
		if( c == 0 )
			return 0;

		if( !IsBlankChar(c) )
		{
			break;
		}
	}
	return c;
}

// Gets a character from the source buffer
// Fill the ci structure with the character information
XmlDocument::CharGiver::ExtractStatus XmlDocument::CharGiver::ExtractChar(int& codePoint, uint32& charLength)
{
	int index = m_state.m_nNextByteIndex;
	
	TextEncoding::ExtractStatus es = m_pTextEncoding->ExtractCodePoint(*m_pRawContent, index, codePoint);
	if( es == TextEncoding::esUnexpectedEndOfString )
	{
		return CharGiver::esUnexpectedEndOfFile;
	}
	if( es != TextEncoding::esNoError )
	{
		return CharGiver::esBadlyEncodedChar;
	}
	if( !IsValidChar(codePoint) )
	{
		return CharGiver::esInvalidCodePoint;
	}

	charLength = index - m_state.m_nNextByteIndex;
	return CharGiver::esNoError;
}

XmlDocument::CharGiver::ExtractStatus XmlDocument::CharGiver::GetNextChar(int& codePoint)
{
	// The encoding should be set with SetEncoding before calling this function
	ASSERT(m_pTextEncoding != nullptr);

	uint32 charLength;
	ExtractStatus eStatus = ExtractChar(codePoint, charLength);
	if( eStatus != esNoError )
		return eStatus;

	// Jump to next char
	m_state.m_nNextByteIndex += charLength;

	///////////////////////////////////////////
	// New line management
	// \r becomes \n, \r\n becomes \n
	if( codePoint == '\n' )
	{
		m_state.m_nLine++;
		return esNoError;
	}
	
	if( codePoint == '\r' )
	{
		if( m_state.m_nNextByteIndex >= m_pRawContent->GetSize() )
		{
			// End of document, the single \r is managed as a single \n
			m_state.m_nLine++;
			codePoint = '\n';
			return esNoError;
		}

		// Check next char
		int codePointNext;
		eStatus = ExtractChar(codePointNext, charLength);
		if( eStatus != esNoError )
			return eStatus;

		if( codePointNext != '\n' )
		{
			// The single \r is managed as a single \n
			m_state.m_nLine++;
			codePoint = '\n';
			return esNoError;
		}
		
		// We have a \r\n sequence, jump to the next real char
		m_state.m_nNextByteIndex += charLength;
		
		m_state.m_nLine++;
		codePoint = '\n'; // Return a single \n (new line)
		return esNoError;
	}
	return esNoError;
}

bool XmlDocument::CharGiver::IsEndOfBufferReached() const
{
	return m_state.m_nNextByteIndex >= m_pRawContent->GetSize();
}

bool XmlDocument::IsValidChar(int codePoint)
{
	if( codePoint <= 0x1f && codePoint != 0x09 && codePoint != 0x0A && codePoint != 0x0D )
	{
		return false;
	}
	if( codePoint > 0x10ffff )
	{
		return false;
	}

	return true;
}

void XmlDocument::RemoveAll()
{
	m_xmlHeader.Empty();
	m_strDocType.Empty();
	m_docnode.m_apChildren.Clear();
}

String XmlDocument::GetLastErrorString() const
{
	wchar szLineNumber[16];
	FormatInt32(m_charGiver.GetCurrentLine(), szLineNumber);

	String str = String("(line ") + String(szLineNumber) + String(") ") + m_strLastError;
	return str;
}
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// Gets all child elements which match the tagname
XmlElementPtrArray XmlElement::GetElementsByTagName(const String& strName)
{
	ASSERT( GetType() == xntElement);

	XmlElementPtrArray aNodes;

	const int32 childCount = m_apChildren.GetSize();
	for(int i = 0; i < childCount; ++i)
	{
		XmlNode* pChild = m_apChildren[i];
		if( pChild->GetType() == xntElement )
		{
			XmlElement* pChildElement = (XmlElement*) pChild;
			if( pChildElement->m_name == strName )
			{
				aNodes.Add(pChildElement);
			}

			aNodes.Add( pChildElement->GetElementsByTagName(strName));
		}
	}
	return aNodes;

}

XmlElementPtrArray XmlElement::GetChildElements(const String& strName)
{
	ASSERT( GetType() == xntElement);

	XmlElementPtrArray aNodes;

	const int32 childCount = m_apChildren.GetSize();
	for(int i = 0; i < childCount; ++i)
	{
		XmlNode* pChild = m_apChildren[i];
		if( pChild->GetType() == xntElement )
		{
			XmlElement* pChildElement = (XmlElement*) pChild;
			if( pChildElement->m_name == strName )
			{
				aNodes.Add(pChildElement);
			}
		}
	}
	return aNodes;
}

ConstXmlElementPtrArray XmlElement::GetChildElements(const String& strName) const
{
	ASSERT( GetType() == xntElement);

	ConstXmlElementPtrArray aNodes;

	const int32 childCount = m_apChildren.GetSize();
	for(int i = 0; i < childCount; ++i)
	{
		const XmlNode* pChild = m_apChildren[i];
		if( pChild->GetType() == xntElement )
		{
			const XmlElement* pChildElement = (const XmlElement*) pChild;
			if( pChildElement->m_name == strName )
			{
				aNodes.Add(pChildElement);
			}
		}
	}
	return aNodes;
}

// Gets the first direct child element
XmlElement* XmlElement::GetFirstElement()
{
	ASSERT( GetType() == xntElement);

	const int32 childCount = m_apChildren.GetSize();
	for(int i = 0; i < childCount; ++i)
	{
		XmlNode* pChild = m_apChildren[i];
		if( pChild->GetType() == xntElement )
		{
			XmlElement* pChildElement = (XmlElement*) pChild;
			return pChildElement;
		}
	}
	return nullptr;
}

XmlNode* XmlElement::GetFirstChild()
{
	ASSERT( GetType() == xntElement);

	if( m_apChildren.GetSize() == 0 )
		return nullptr;

	return m_apChildren[0];
}

// Gets the first text of this element
chustd::String XmlElement::GetFirstText()
{
	ASSERT( GetType() == xntElement);

	const int32 childCount = m_apChildren.GetSize();
	for(int i = 0; i < childCount; ++i)
	{
		XmlNode* pChild = m_apChildren[i];
		if( pChild->GetType() == xntText )
		{
			XmlText* pChildText = (XmlText*) pChild;
			return pChildText->m_strData;
		}
	}

	return "";
}

// Gets the attribute value
chustd::String XmlElement::GetAttribute(const String& strName) const
{
	ASSERT( GetType() == xntElement);

	const int32 attCount = m_aAttributes.GetSize();
	for(int i = 0; i < attCount; ++i)
	{
		if( m_aAttributes[i].m_name == strName )
		{
			return m_aAttributes[i].m_strValue;
		}
	}
	return "";
}

//////////////////////////////////////////////////////////////////////////////////////////////
bool XmlDocument::UpdateIdMap()
{
	return UpdateIdMap(&m_docnode);
}

bool XmlDocument::UpdateIdMap(XmlElement* pStartingElement)
{
	if( pStartingElement == nullptr )
		return true;

	ASSERT( pStartingElement->GetType() == xntElement);

	String strId = pStartingElement->GetAttribute("id");
	if( !strId.IsEmpty() )
	{
		// Add this id to the id list
		AddIdInformation(strId, pStartingElement);
	}

	// Recursive call on children
	PtrArray<XmlNode>& apChildrenNodes = pStartingElement->m_apChildren;

	int32 childrenCount = apChildrenNodes.GetSize();
	for(int i = 0; i < childrenCount; ++i)
	{
		XmlNode* pChildNode = apChildrenNodes[i];

		// We only work with elements
		if(pChildNode->GetType() != xntElement)
			continue;

		if( !UpdateIdMap((XmlElement*)pChildNode) )
			return false;
	}

	return true;
}

bool XmlDocument::AddIdInformation(const String& strId, XmlElement* pElement)
{
	ASSERT( pElement->GetType() == xntElement);

	// Check if the id already exists
	XmlElement* pExistingElement = GetElementById(strId);

	if( pExistingElement )
	{
		m_strLastError = "Id " + strId + " already exists";
		return false;
	}

	// Add the id to the list
	int index = m_aIds.Add();
	IdMapping& mapping = m_aIds[index];
	mapping.m_strId = strId;
	mapping.m_pNodeElement = pElement;

	return true;
}

XmlElement* XmlDocument::GetElementById(const String& strId)
{
	int32 nIdCount = m_aIds.GetSize();
	for(int i = 0; i < nIdCount; ++i)
	{
		IdMapping& mapping = m_aIds[i];
		if(mapping.m_strId == strId)
		{
			return mapping.m_pNodeElement;
		}
	}

	return nullptr;
}

