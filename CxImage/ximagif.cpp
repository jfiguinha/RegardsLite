/*
 * File:	ximagif.cpp
 * Purpose:	Platform Independent GIF Image Class Loader and Writer
 * 07/Aug/2001 Davide Pizzolato - www.xdp.it
 * CxImage version 7.0.1 07/Jan/2011
 */

#include "ximagif.h"

#if CXIMAGE_SUPPORT_GIF

#include "ximaiter.h"

#if defined (_WIN32_WCE)
	#define assert(s)
#else
#include <assert.h>
#endif

////////////////////////////////////////////////////////////////////////////////
CxImageGIF::CxImageGIF(): CxImage(CXIMAGE_FORMAT_GIF)
{
	buf = new uint8_t [GIFBUFTAM + 1];

	stack = new uint8_t [MAX_CODES + 1];
	suffix = new uint8_t [MAX_CODES + 1];
	prefix = new uint16_t [MAX_CODES + 1];

	htab = new int32_t [HSIZE];
	codetab = new uint16_t [HSIZE];

	byte_buff = new uint8_t [257];
	accum = new char [256];
	m_comment = new char [256];

	m_loops = 0;
	info.dispmeth = 0;
	m_comment[0] = '\0';
}

////////////////////////////////////////////////////////////////////////////////
CxImageGIF::~CxImageGIF()
{
	delete [] buf;

	delete [] stack;
	delete [] suffix;
	delete [] prefix;

	delete [] htab;
	delete [] codetab;

	delete [] byte_buff;
	delete [] accum;
	delete [] m_comment;
}

////////////////////////////////////////////////////////////////////////////////
#if CXIMAGE_SUPPORT_DECODE
////////////////////////////////////////////////////////////////////////////////
bool CxImageGIF::Decode(CxFile* fp)
{
	/* AD - for transparency */
	struct_dscgif dscgif;
	struct_image image;
	struct_TabCol TabCol;

	if (fp == nullptr) return false;

	fp->Read(&dscgif,/*sizeof(dscgif)*/13, 1);
	//if (strncmp(dscgif.header,"GIF8",3)!=0) {
	if (strncmp(dscgif.header, "GIF8", 4) != 0) return FALSE;

	// Avoid Byte order problem with Mac <AMSN>
	dscgif.scrheight = m_ntohs(dscgif.scrheight);
	dscgif.scrwidth = m_ntohs(dscgif.scrwidth);

	if (info.nEscape == -1)
	{
		// Return output dimensions only
		head.biWidth = dscgif.scrwidth;
		head.biHeight = dscgif.scrheight;
		info.dwType = CXIMAGE_FORMAT_GIF;
		return true;
	}

	/* AD - for interlace */
	TabCol.sogct = static_cast<int16_t>(1 << ((dscgif.pflds & 0x07) + 1));
	TabCol.colres = static_cast<int16_t>(((dscgif.pflds & 0x70) >> 4) + 1);

	// assume that the image is a truecolor-gif if
	// 1) no global color map found
	// 2) (image.w, image.h) of the 1st image != (dscgif.scrwidth, dscgif.scrheight)
	int32_t bTrueColor = 0;
	CxImage* imaRGB = nullptr;

	// Global colour map?
	if (dscgif.pflds & 0x80)
		fp->Read(TabCol.paleta, sizeof(struct rgb_color) * TabCol.sogct, 1);
	else
		bTrueColor++; //first chance for a truecolor gif

	int32_t first_transparent_index = 0;

	int32_t iImage = 0;
	info.nNumFrames = get_num_frames(fp, &TabCol, &dscgif);

	if ((info.nFrame < 0) || (info.nFrame >= info.nNumFrames)) return false;

	//it cannot be a true color GIF with only one frame
	if (info.nNumFrames == 1)
		bTrueColor = 0;

	char ch;
	bool bPreviousWasNull = true;
	int32_t prevdispmeth = 0;
	CxImage* previousFrame = nullptr;

	for (BOOL bContinue = TRUE; bContinue;)
	{
		if (fp->Read(&ch, sizeof(ch), 1) != 1) { break; }

		if (info.nEscape > 0) return false; // <vho> - cancel decoding
		if (bPreviousWasNull || ch == 0)
		{
			switch (ch)
			{
			case '!': // extension
				{
					bContinue = DecodeExtension(fp);
					break;
				}
			case ',': // image
				{
					assert(sizeof(image) == 9);
					fp->Read(&image, sizeof(image), 1);
					//avoid byte order problems with Solaris <candan> <AMSN>
					image.l = m_ntohs(image.l);
					image.t = m_ntohs(image.t);
					image.w = m_ntohs(image.w);
					image.h = m_ntohs(image.h);

					if (((image.l + image.w) > dscgif.scrwidth) || ((image.t + image.h) > dscgif.scrheight))
						break;

					// check if it could be a truecolor gif
					if ((iImage == 0) && (image.w != dscgif.scrwidth) && (image.h != dscgif.scrheight))
						bTrueColor++;

					rgb_color locpal[256]; //Local Palette 
					rgb_color* pcurpal = TabCol.paleta; //Current Palette 
					int16_t palcount = TabCol.sogct; //Current Palette color count  

					// Local colour map?
					if (image.pf & 0x80)
					{
						palcount = static_cast<int16_t>(1 << ((image.pf & 0x07) + 1));
						assert(3 == sizeof(struct rgb_color));
						fp->Read(locpal, sizeof(struct rgb_color) * palcount, 1);
						pcurpal = locpal;
					}

					int32_t bpp; //<DP> select the correct bit per pixel value
					if (palcount <= 2) bpp = 1;
					else if (palcount <= 16) bpp = 4;
					else bpp = 8;

					CxImageGIF backimage;
					backimage.CopyInfo(*this);
					if (iImage == 0)
					{
						//first frame: build image background
						backimage.Create(dscgif.scrwidth, dscgif.scrheight, bpp, CXIMAGE_FORMAT_GIF);
						first_transparent_index = info.nBkgndIndex;
						backimage.Clear(gifgce.transpcolindex);
						previousFrame = new CxImage(backimage);
						previousFrame->SetRetreiveAllFrames(false);
					}
					else
					{
						//generic frame: handle disposal method from previous one
						/*Values :  0 -   No disposal specified. The decoder is
										  not required to take any action.
									1 -   Do not dispose. The graphic is to be left
										  in place.
									2 -   Restore to background color. The area used by the
										  graphic must be restored to the background color.
									3 -   Restore to previous. The decoder is required to
										  restore the area overwritten by the graphic with
										  what was there prior to rendering the graphic.
						*/
						/*	backimage.Copy(*this);
							if (prevdispmeth==2){
								backimage.Clear((uint8_t)first_transparent_index);
							}*/
						if (prevdispmeth == 2)
						{
							backimage.Copy(*this, false, false, false);
							backimage.Clear(static_cast<uint8_t>(first_transparent_index));
						}
						else if (prevdispmeth == 3)
						{
							backimage.Copy(*this, false, false, false);
							backimage.Create(previousFrame->GetWidth(),
							                 previousFrame->GetHeight(),
							                 previousFrame->GetBpp(), CXIMAGE_FORMAT_GIF);
							memcpy(backimage.GetDIB(), previousFrame->GetDIB(),
							       backimage.GetSize());
							//backimage.AlphaSet(*previousFrame);
						}
						else
						{
							backimage.Copy(*this);
						}
					}

					//active frame
					Create(image.w, image.h, bpp, CXIMAGE_FORMAT_GIF);

					if ((image.pf & 0x80) || (dscgif.pflds & 0x80))
					{
						uint8_t r[256], g[256], b[256];
						int32_t i, has_white = 0;

						for (i = 0; i < palcount; i++)
						{
							r[i] = pcurpal[i].r;
							g[i] = pcurpal[i].g;
							b[i] = pcurpal[i].b;
							if (RGB(r[i], g[i], b[i]) == 0xFFFFFF) has_white = 1;
						}

						// Force transparency colour white...
						//if (0) if (info.nBkgndIndex >= 0)
						//	r[info.nBkgndIndex] = g[info.nBkgndIndex] = b[info.nBkgndIndex] = 255;
						// Fill in with white // AD
						if (info.nBkgndIndex >= 0)
						{
							while (i < 256)
							{
								has_white = 1;
								r[i] = g[i] = b[i] = 255;
								i++;
							}
						}

						// Force last colour to white...   // AD
						//if ((info.nBkgndIndex >= 0) && !has_white) {
						//	r[255] = g[255] = b[255] = 255;
						//}

						SetPalette((info.nBkgndIndex >= 0 ? 256 : palcount), r, g, b);
					}

					auto iter = new CImageIterator(this);
					iter->Upset();
					int32_t badcode = 0;
					ibf = GIFBUFTAM + 1;

					interlaced = image.pf & 0x40;
					iheight = image.h;
					istep = 8;
					iypos = 0;
					ipass = 0;

					int32_t pos_start = fp->Tell();
					//if (interlaced) log << "Interlaced" << endl;
					decoder(fp, iter, image.w, badcode);
					delete iter;

					if (info.nEscape) return false; // <vho> - cancel decoding

					if (bTrueColor < 2)
					{
						//standard GIF: mix frame with background
						backimage.IncreaseBpp(bpp);
						backimage.GifMix(*this, image);
						backimage.SetTransIndex(first_transparent_index);
						backimage.SetPalette(GetPalette());
						Transfer(backimage, false);
					}
					else
					{
						//it's a truecolor gif!
						//force full image decoding
						info.nFrame = info.nNumFrames - 1;
						//build the RGB image
						if (imaRGB == nullptr)
							imaRGB = new CxImage(dscgif.scrwidth, dscgif.scrheight, 24,
							                     CXIMAGE_FORMAT_GIF);
						//copy the partial image into the full RGB image
						for (int32_t y = 0; y < image.h; y++)
						{
							for (int32_t x = 0; x < image.w; x++)
							{
								imaRGB->SetPixelColor(x + image.l, dscgif.scrheight - 1 - image.t - y,
								                      GetPixelColor(x, image.h - y - 1));
							}
						}
					}

					prevdispmeth = (gifgce.flags >> 2) & 0x7;

					//restore the correct position in the file for the next image
					if (badcode)
					{
						seek_next_image(fp, pos_start);
					}
					else
					{
						fp->Seek(-(ibfmax - ibf - 1), SEEK_CUR);
					}

					if (info.bGetAllFrames && imaRGB == nullptr)
					{
						if (iImage == 0)
						{
							DestroyFrames();
							ppFrames = new CxImage*[info.nNumFrames];
							for (int32_t frameIdx = 0; frameIdx < info.nNumFrames; frameIdx++)
							{
								ppFrames[frameIdx] = nullptr;
							}
						}
						ppFrames[iImage] = new CxImage(*this);
						ppFrames[iImage]->SetRetreiveAllFrames(false);
					}
					if (prevdispmeth <= 1)
					{
						delete previousFrame;
						previousFrame = new CxImage(*this);
						previousFrame->SetRetreiveAllFrames(false);
					}

					if ((info.nFrame == iImage) && (info.bGetAllFrames == false)) bContinue = false;
					else iImage++;

					break;
				}
			case ';': //terminator
				bContinue = false;
				break;
			default:
				bPreviousWasNull = (ch == 0);
				break;
			}
		}
	}

	if (bTrueColor >= 2 && imaRGB)
	{
		if (gifgce.flags & 0x1)
		{
			imaRGB->SetTransColor(GetPaletteColor(static_cast<uint8_t>(info.nBkgndIndex)));
			imaRGB->SetTransIndex(0);
		}
		Transfer(*imaRGB);
	}
	delete imaRGB;

	delete previousFrame;

	return true;
}

////////////////////////////////////////////////////////////////////////////////
bool CxImageGIF::DecodeExtension(CxFile* fp)
{
	bool bContinue;
	uint8_t count;
	uint8_t fc;

	bContinue = (1 == fp->Read(&fc, sizeof(fc), 1));
	if (bContinue)
	{
		/* AD - for transparency */
		if (fc == 0xF9)
		{
			bContinue = (1 == fp->Read(&count, sizeof(count), 1));
			if (bContinue)
			{
				assert(sizeof(gifgce) == 4);
				bContinue = (count == fp->Read(&gifgce, 1, sizeof(gifgce)));
				gifgce.delaytime = m_ntohs(gifgce.delaytime); // Avoid Byte order problem with Mac <AMSN>
				if (bContinue)
				{
					info.nBkgndIndex = (gifgce.flags & 0x1) ? gifgce.transpcolindex : -1;
					info.dwFrameDelay = gifgce.delaytime;
					SetDisposalMethod((gifgce.flags >> 2) & 0x7);
				}
			}
		}

		if (fc == 0xFE)
		{
			//<DP> Comment block
			bContinue = (1 == fp->Read(&count, sizeof(count), 1));
			if (bContinue)
			{
				bContinue = (1 == fp->Read(m_comment, count, 1));
				m_comment[count] = '\0';
			}
		}

		if (fc == 0xFF)
		{
			//<DP> Application Extension block
			bContinue = (1 == fp->Read(&count, sizeof(count), 1));
			if (bContinue)
			{
				bContinue = (count == 11);
				if (bContinue)
				{
					char AppID[11];
					bContinue = (1 == fp->Read(AppID, count, 1));
					if (bContinue)
					{
						bContinue = (1 == fp->Read(&count, sizeof(count), 1));
						if (bContinue)
						{
							auto dati = static_cast<uint8_t*>(malloc(count));
							bContinue = (dati != nullptr);
							if (bContinue)
							{
								bContinue = (1 == fp->Read(dati, count, 1));
								if (count > 2)
								{
									m_loops = dati[1] + 256 * dati[2];
								}
							}
							free(dati);
						}
					}
				}
			}
		}

		while (bContinue && fp->Read(&count, sizeof(count), 1) && count)
		{
			//log << "Skipping " << count << " bytes" << endl;
			fp->Seek(count, SEEK_CUR);
		}
	}
	return bContinue;
}

////////////////////////////////////////////////////////////////////////////////
#endif //CXIMAGE_SUPPORT_DECODE
////////////////////////////////////////////////////////////////////////////////

//   - This external (machine specific) function is expected to return
// either the next uint8_t from the GIF file, or a negative error number.
int32_t CxImageGIF::get_byte(CxFile* file)
{
	if (ibf >= GIFBUFTAM)
	{
		// FW 06/02/98 >>>
		ibfmax = static_cast<int32_t>(file->Read(buf, 1, GIFBUFTAM));
		if (ibfmax < GIFBUFTAM) buf[ibfmax] = 255;
		// FW 06/02/98 <<<
		ibf = 0;
	}
	if (ibf >= ibfmax) return -1; //<DP> avoid overflows
	return buf[ibf++];
}

////////////////////////////////////////////////////////////////////////////////
/*   - This function takes a full line of pixels (one uint8_t per pixel) and
 * displays them (or does whatever your program wants with them...).  It
 * should return zero, or negative if an error or some other event occurs
 * which would require aborting the decode process...  Note that the length
 * passed will almost always be equal to the line length passed to the
 * decoder function, with the sole exception occurring when an ending code
 * occurs in an odd place in the GIF file...  In any case, linelen will be
 * equal to the number of pixels passed...
*/
int32_t CxImageGIF::out_line(CImageIterator* iter, uint8_t* pixels, int32_t linelen)
{
	if (iter == nullptr || pixels == nullptr)
		return -1;

	//<DP> for 1 & 4 bpp images, the pixels are compressed
	if (head.biBitCount < 8)
	{
		for (int32_t x = 0; x < head.biWidth; x++)
		{
			uint8_t pos;
			uint8_t* iDst = pixels + (x * head.biBitCount >> 3);
			if (head.biBitCount == 4)
			{
				pos = static_cast<uint8_t>(4 * (1 - x % 2));
				*iDst &= ~(0x0F << pos);
				*iDst |= ((pixels[x] & 0x0F) << pos);
			}
			else if (head.biBitCount == 1)
			{
				pos = static_cast<uint8_t>(7 - x % 8);
				*iDst &= ~(0x01 << pos);
				*iDst |= ((pixels[x] & 0x01) << pos);
			}
		}
	}

	/* AD - for interlace */
	if (interlaced)
	{
		iter->SetY(iheight - iypos - 1);
		iter->SetRow(pixels, linelen);

		if ((iypos += istep) >= iheight)
		{
			do
			{
				if (ipass++ > 0) istep /= 2;
				iypos = istep / 2;
			}
			while (iypos > iheight);
		}
		return 0;
	}
	if (iter->ItOK())
	{
		iter->SetRow(pixels, linelen);
		(void)iter->PrevRow();
		return 0;
	}
	//	 puts("chafeo");
	return -1;
}

////////////////////////////////////////////////////////////////////////////////
#if CXIMAGE_SUPPORT_ENCODE
////////////////////////////////////////////////////////////////////////////////
// SaveFile - writes GIF87a gif file
// Randy Spann 6/15/97
// R.Spann@ConnRiver.net
bool CxImageGIF::Encode(CxFile* fp)
{
	if (EncodeSafeCheck(fp)) return false;

	if (head.biBitCount > 8)
	{
		//strcpy(info.szLastError,"GIF Images must be 8 bit or less");
		//return FALSE;
		return EncodeRGB(fp);
	}

	if (GetNumFrames() > 1 && ppFrames)
	{
		return Encode(fp, ppFrames, GetNumFrames());
	}

	EncodeHeader(fp);

	EncodeExtension(fp);

	EncodeComment(fp);

	EncodeBody(fp);

	fp->PutC(';'); // Write the GIF file terminator

	return true; // done!
}

////////////////////////////////////////////////////////////////////////////////
bool CxImageGIF::Encode(CxFile* fp, CxImage** pImages, int32_t pagecount, bool bLocalColorMap, bool bLocalDispMeth)
{
	cx_try
	{
		if (fp == nullptr)
			cx_throw("invalid file pointer");
		if (pImages == nullptr || pagecount <= 0 || pImages[0] == nullptr)
			cx_throw("multipage GIF, no images!");

		int32_t i;
		for (i = 0; i < pagecount; i++)
		{
			if (pImages[i] == nullptr)
				cx_throw("Bad image pointer");
			if (!(pImages[i]->IsValid()))
				cx_throw("Empty image");
			if (pImages[i]->GetNumColors() == 0)
				cx_throw("CxImageGIF::Encode cannot create animated GIFs with a true color frame. Use DecreaseBpp before");
		}

		CxImageGIF ghost;

		//write the first image
		ghost.Ghost(pImages[0]);
		ghost.EncodeHeader(fp);

		if (m_loops != 1)
		{
			ghost.SetLoops(max(0, m_loops-1));
			ghost.EncodeLoopExtension(fp);
		}

		if (bLocalDispMeth)
		{
			ghost.EncodeExtension(fp);
		}
		else
		{
			uint8_t dm = ghost.GetDisposalMethod();
			ghost.SetDisposalMethod(GetDisposalMethod());
			ghost.EncodeExtension(fp);
			ghost.SetDisposalMethod(dm);
		}

		EncodeComment(fp);

		ghost.EncodeBody(fp);

		for (i = 1; i < pagecount; i++)
		{
			ghost.Ghost(pImages[i]);

			if (bLocalDispMeth)
			{
				ghost.EncodeExtension(fp);
			}
			else
			{
				uint8_t dm = ghost.GetDisposalMethod();
				ghost.SetDisposalMethod(GetDisposalMethod());
				ghost.EncodeExtension(fp);
				ghost.SetDisposalMethod(dm);
			}

			ghost.EncodeBody(fp, bLocalColorMap);
		}

		fp->PutC(';'); // Write the GIF file terminator
	} cx_catch
	{
		if (strcmp(message, "")) strncpy(info.szLastError, message, 255);
		return false;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////
void CxImageGIF::EncodeHeader(CxFile* fp)
{
	fp->Write("GIF89a", 1, 6); //GIF Header

	Putword(head.biWidth, fp); //Logical screen descriptor
	Putword(head.biHeight, fp);

	uint8_t Flags;
	if (head.biClrUsed == 0)
	{
		Flags = 0x11;
	}
	else
	{
		Flags = 0x80;
		Flags |= (head.biBitCount - 1) << 5;
		Flags |= (head.biBitCount - 1);
	}

	fp->PutC(Flags); //GIF "packed fields"
	fp->PutC(0); //GIF "BackGround"
	fp->PutC(0); //GIF "pixel aspect ratio"

	if (head.biClrUsed != 0)
	{
		RGBQUAD* pPal = GetPalette();
		for (uint32_t i = 0; i < head.biClrUsed; ++i)
		{
			fp->PutC(pPal[i].rgbRed);
			fp->PutC(pPal[i].rgbGreen);
			fp->PutC(pPal[i].rgbBlue);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
void CxImageGIF::EncodeExtension(CxFile* fp)
{
	// TRK BEGIN : transparency
	fp->PutC('!');
	fp->PutC(TRANSPARENCY_CODE);

	gifgce.flags = 0;
	gifgce.flags |= ((info.nBkgndIndex != -1) ? 1 : 0);
	gifgce.flags |= ((GetDisposalMethod() & 0x7) << 2);
	gifgce.delaytime = static_cast<uint16_t>(info.dwFrameDelay);
	gifgce.transpcolindex = static_cast<uint8_t>(info.nBkgndIndex);

	//Invert byte order in case we use a byte order arch, then set it back <AMSN>
	gifgce.delaytime = m_ntohs(gifgce.delaytime);
	fp->PutC(sizeof(gifgce));
	fp->Write(&gifgce, sizeof(gifgce), 1);
	gifgce.delaytime = m_ntohs(gifgce.delaytime);

	fp->PutC(0);
	// TRK END
}

////////////////////////////////////////////////////////////////////////////////
void CxImageGIF::EncodeLoopExtension(CxFile* fp)
{
	fp->PutC('!'); //byte  1  : 33 (hex 0x21) GIF Extension code
	fp->PutC(255); //byte  2  : 255 (hex 0xFF) Application Extension Label
	fp->PutC(11); //byte  3  : 11 (hex (0x0B) Length of Application Block (eleven bytes of data to follow)
	fp->Write("NETSCAPE2.0", 11, 1);
	fp->PutC(3); //byte 15  : 3 (hex 0x03) Length of Data Sub-Block (three bytes of data to follow)
	fp->PutC(1); //byte 16  : 1 (hex 0x01)
	Putword(m_loops, fp); //bytes 17 to 18 : 0 to 65535, an unsigned integer in lo-hi byte format. 
	//This indicate the number of iterations the loop should be executed.
	fp->PutC(0); //bytes 19       : 0 (hex 0x00) a Data Sub-block Terminator. 
}

////////////////////////////////////////////////////////////////////////////////
void CxImageGIF::EncodeBody(CxFile* fp, bool bLocalColorMap)
{
	curx = 0;
	cury = head.biHeight - 1; //because we read the image bottom to top
	CountDown = static_cast<int32_t>(head.biWidth) * static_cast<int32_t>(head.biHeight);

	fp->PutC(',');

	Putword(info.xOffset, fp);
	Putword(info.yOffset, fp);
	Putword(head.biWidth, fp);
	Putword(head.biHeight, fp);

	uint8_t Flags = 0x00; //non-interlaced (0x40 = interlaced) (0x80 = LocalColorMap)
	if (bLocalColorMap)
	{
		Flags |= 0x80;
		Flags |= head.biBitCount - 1;
	}
	fp->PutC(Flags);

	if (bLocalColorMap)
	{
		Flags |= 0x87;
		RGBQUAD* pPal = GetPalette();
		for (uint32_t i = 0; i < head.biClrUsed; ++i)
		{
			fp->PutC(pPal[i].rgbRed);
			fp->PutC(pPal[i].rgbGreen);
			fp->PutC(pPal[i].rgbBlue);
		}
	}

	int32_t InitCodeSize = head.biBitCount <= 1 ? 2 : head.biBitCount;
	// Write out the initial code size
	fp->PutC(static_cast<uint8_t>(InitCodeSize));

	// Go and actually compress the data
	switch (GetCodecOption(CXIMAGE_FORMAT_GIF))
	{
	case 1: //uncompressed
		compressNONE(InitCodeSize + 1, fp);
		break;
	case 2: //RLE
		compressRLE(InitCodeSize + 1, fp);
		break;
	default: //LZW
		compressLZW(InitCodeSize + 1, fp);
	}

	// Write out a Zero-length packet (to end the series)
	fp->PutC(0);
}

////////////////////////////////////////////////////////////////////////////////
void CxImageGIF::EncodeComment(CxFile* fp)
{
	uint32_t n = static_cast<uint32_t>(strlen(m_comment));
	if (n > 255) n = 255;
	if (n)
	{
		fp->PutC('!'); //extension code:
		fp->PutC(254); //comment extension
		fp->PutC(static_cast<uint8_t>(n)); //size of comment
		fp->Write(m_comment, n, 1);
		fp->PutC(0); //block terminator
	}
}

////////////////////////////////////////////////////////////////////////////////
bool CxImageGIF::EncodeRGB(CxFile* fp)
{
	EncodeHeader(fp);

	//	EncodeLoopExtension(fp);

	EncodeComment(fp);

	uint32_t w, h;
	w = h = 0;
	const int32_t cellw = 17;
	const int32_t cellh = 15;
	CxImageGIF tmp;
	for (int32_t y = 0; y < head.biHeight; y += cellh)
	{
		for (int32_t x = 0; x < head.biWidth; x += cellw)
		{
			if ((head.biWidth - x) < cellw) w = head.biWidth - x;
			else w = cellw;
			if ((head.biHeight - y) < cellh) h = head.biHeight - y;
			else h = cellh;

			if (w != tmp.GetWidth() || h != tmp.GetHeight()) tmp.Create(w, h, 8);

			if (IsTransparent())
			{
				tmp.SetTransIndex(0);
				tmp.SetPaletteColor(0, GetTransColor());
			}

			uint8_t i;
			for (uint32_t j = 0; j < h; j++)
			{
				for (uint32_t k = 0; k < w; k++)
				{
					i = static_cast<uint8_t>(1 + k + cellw * j);
					tmp.SetPaletteColor(i, GetPixelColor(x + k, head.biHeight - y - h + j));
					tmp.SetPixelIndex(k, j, tmp.GetNearestIndex(tmp.GetPaletteColor(i)));
				}
			}

			tmp.SetOffset(x, y);
			tmp.EncodeExtension(fp);
			tmp.EncodeBody(fp, true);
		}
	}

	fp->PutC(';'); // Write the GIF file terminator

	return true; // done!
}

////////////////////////////////////////////////////////////////////////////////
#endif // CXIMAGE_SUPPORT_ENCODE
////////////////////////////////////////////////////////////////////////////////
// Return the next pixel from the image
// <DP> fix for 1 & 4 bpp images
int32_t CxImageGIF::GifNextPixel()
{
	if (CountDown == 0) return EOF;
	--CountDown;
	int32_t r = GetPixelIndex(curx, cury);
	// Bump the current X position
	++curx;
	if (curx == head.biWidth)
	{
		curx = 0;
		cury--; //bottom to top
	}
	return r;
}

////////////////////////////////////////////////////////////////////////////////
void CxImageGIF::Putword(int32_t w, CxFile* fp)
{
	fp->PutC(static_cast<uint8_t>(w & 0xff));
	fp->PutC(static_cast<uint8_t>((w >> 8) & 0xff));
}

////////////////////////////////////////////////////////////////////////////////
void CxImageGIF::compressNONE(int32_t init_bits, CxFile* outfile)
{
	int32_t c;
	int32_t ent;

	// g_init_bits - initial number of bits
	// g_outfile   - pointer to output file
	g_init_bits = init_bits;
	g_outfile = outfile;

	// Set up the necessary values
	cur_accum = cur_bits = clear_flg = 0;
	maxcode = static_cast<int16_t>(MAXCODE(n_bits = g_init_bits));
	code_int maxmaxcode = static_cast<code_int>(1) << MAXBITSCODES;

	ClearCode = (1 << (init_bits - 1));
	EOFCode = ClearCode + 1;
	free_ent = static_cast<int16_t>(ClearCode + 2);

	a_count = 0;
	ent = GifNextPixel();

	output(static_cast<code_int>(ClearCode));

	while (ent != EOF)
	{
		c = GifNextPixel();

		output(static_cast<code_int>(ent));
		ent = c;
		if (free_ent < maxmaxcode)
		{
			free_ent++;
		}
		else
		{
			free_ent = static_cast<int16_t>(ClearCode + 2);
			clear_flg = 1;
			output(static_cast<code_int>(ClearCode));
		}
	}
	// Put out the final code.
	output(static_cast<code_int>(EOFCode));
}

////////////////////////////////////////////////////////////////////////////////

/***************************************************************************
 *
 *  GIFCOMPR.C       -     LZW GIF Image compression routines
 *
 ***************************************************************************/

void CxImageGIF::compressLZW(int32_t init_bits, CxFile* outfile)
{
	int32_t fcode;
	int32_t c;
	int32_t ent;
	int32_t hshift;
	int32_t disp;
	int32_t i;

	// g_init_bits - initial number of bits
	// g_outfile   - pointer to output file
	g_init_bits = init_bits;
	g_outfile = outfile;

	// Set up the necessary values
	cur_accum = cur_bits = clear_flg = 0;
	maxcode = static_cast<int16_t>(MAXCODE(n_bits = g_init_bits));
	code_int maxmaxcode = static_cast<code_int>(1) << MAXBITSCODES;

	ClearCode = (1 << (init_bits - 1));
	EOFCode = ClearCode + 1;
	free_ent = static_cast<int16_t>(ClearCode + 2);

	a_count = 0;
	ent = GifNextPixel();

	hshift = 0;
	for (fcode = HSIZE; fcode < 65536L; fcode *= 2L) ++hshift;
	hshift = 8 - hshift; /* set hash code range bound */
	cl_hash(HSIZE); /* clear hash table */
	output(static_cast<code_int>(ClearCode));

	while ((c = GifNextPixel()) != EOF)
	{
		fcode = (c << MAXBITSCODES) + ent;
		i = ((static_cast<code_int>(c) << hshift) ^ ent); /* xor hashing */

		if (HashTabOf(i) == fcode)
		{
			ent = CodeTabOf(i);
			continue;
		}
		if (HashTabOf(i) < 0) /* empty slot */
			goto nomatch;
		disp = HSIZE - i; /* secondary hash (after G. Knott) */
		if (i == 0) disp = 1;
	probe:
		if ((i -= disp) < 0) i += HSIZE;
		if (HashTabOf(i) == fcode)
		{
			ent = CodeTabOf(i);
			continue;
		}
		if (HashTabOf(i) > 0) goto probe;
	nomatch:
		output(static_cast<code_int>(ent));
		ent = c;
		if (free_ent < maxmaxcode)
		{
			CodeTabOf(i) = free_ent++; /* code -> hashtable */
			HashTabOf(i) = fcode;
		}
		else
		{
			cl_hash(HSIZE);
			free_ent = static_cast<int16_t>(ClearCode + 2);
			clear_flg = 1;
			output(static_cast<code_int>(ClearCode));
		}
	}
	// Put out the final code.
	output(static_cast<code_int>(ent));
	output(static_cast<code_int>(EOFCode));
}

////////////////////////////////////////////////////////////////////////////////

static const uint32_t code_mask[] = {
	0x0000, 0x0001, 0x0003, 0x0007, 0x000F,
	0x001F, 0x003F, 0x007F, 0x00FF,
	0x01FF, 0x03FF, 0x07FF, 0x0FFF,
	0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF
};

////////////////////////////////////////////////////////////////////////////////
void CxImageGIF::output(code_int code)
{
	cur_accum &= code_mask[cur_bits];

	if (cur_bits > 0)
		cur_accum |= (static_cast<int32_t>(code) << cur_bits);
	else
		cur_accum = code;

	cur_bits += n_bits;

	while (cur_bits >= 8)
	{
		char_out(cur_accum & 0xff);
		cur_accum >>= 8;
		cur_bits -= 8;
	}

	/*
	 * If the next entry is going to be too big for the code size,
	 * then increase it, if possible.
	 */

	if (free_ent > maxcode || clear_flg)
	{
		if (clear_flg)
		{
			maxcode = static_cast<int16_t>(MAXCODE(n_bits = g_init_bits));
			clear_flg = 0;
		}
		else
		{
			++n_bits;
			if (n_bits == MAXBITSCODES)
				maxcode = static_cast<code_int>(1) << MAXBITSCODES; /* should NEVER generate this code */
			else
				maxcode = static_cast<int16_t>(MAXCODE(n_bits));
		}
	}

	if (code == EOFCode)
	{
		// At EOF, write the rest of the buffer.
		while (cur_bits > 0)
		{
			char_out(cur_accum & 0xff);
			cur_accum >>= 8;
			cur_bits -= 8;
		}

		flush_char();
		g_outfile->Flush();

		if (g_outfile->Error()) strcpy(info.szLastError, "Write Error in GIF file");
	}
}

////////////////////////////////////////////////////////////////////////////////

void CxImageGIF::cl_hash(int32_t hsize)

{
	int32_t* htab_p = htab + hsize;

	int32_t i;
	int32_t m1 = -1L;

	i = hsize - 16;

	do
	{
		*(htab_p - 16) = m1;
		*(htab_p - 15) = m1;
		*(htab_p - 14) = m1;
		*(htab_p - 13) = m1;
		*(htab_p - 12) = m1;
		*(htab_p - 11) = m1;
		*(htab_p - 10) = m1;
		*(htab_p - 9) = m1;
		*(htab_p - 8) = m1;
		*(htab_p - 7) = m1;
		*(htab_p - 6) = m1;
		*(htab_p - 5) = m1;
		*(htab_p - 4) = m1;
		*(htab_p - 3) = m1;
		*(htab_p - 2) = m1;
		*(htab_p - 1) = m1;

		htab_p -= 16;
	}
	while ((i -= 16) >= 0);

	for (i += 16; i > 0; --i)
		*--htab_p = m1;
}

/*******************************************************************************
*   GIF specific
*******************************************************************************/

void CxImageGIF::char_out(int32_t c)
{
	accum[a_count++] = static_cast<char>(c);
	if (a_count >= 254)
		flush_char();
}

void CxImageGIF::flush_char()
{
	if (a_count > 0)
	{
		g_outfile->PutC(static_cast<uint8_t>(a_count));
		g_outfile->Write(accum, 1, a_count);
		a_count = 0;
	}
}

/*******************************************************************************
*   GIF decoder
*******************************************************************************/
/* DECODE.C - An LZW decoder for GIF
 * Copyright (C) 1987, by Steven A. Bennett
 * Copyright (C) 1994, C++ version by Alejandro Aguilar Sierra
*
 * Permission is given by the author to freely redistribute and include
 * this code in any program as int32_t as this credit is given where due.
 *
 * In accordance with the above, I want to credit Steve Wilhite who wrote
 * the code which this is heavily inspired by...
 *
 * GIF and 'Graphics Interchange Format' are trademarks (tm) of
 * Compuserve, Incorporated, an H&R Block Company.
 *
 * Release Notes: This file contains a decoder routine for GIF images
 * which is similar, structurally, to the original routine by Steve Wilhite.
 * It is, however, somewhat noticably faster in most cases.
 *
 */

#if CXIMAGE_SUPPORT_DECODE
////////////////////////////////////////////////////////////////////////////////

int16_t CxImageGIF::init_exp(int16_t size)
{
	curr_size = static_cast<int16_t>(size + 1);
	top_slot = static_cast<int16_t>(1 << curr_size);
	clear = static_cast<int16_t>(1 << size);
	ending = static_cast<int16_t>(clear + 1);
	slot = newcodes = static_cast<int16_t>(ending + 1);
	navail_bytes = nbits_left = 0;

	memset(stack, 0,MAX_CODES + 1);
	memset(prefix, 0,MAX_CODES + 1);
	memset(suffix, 0,MAX_CODES + 1);
	return (0);
}

////////////////////////////////////////////////////////////////////////////////

/* get_next_code()
 * - gets the next code from the GIF file.  Returns the code, or else
 * a negative number in case of file errors...
 */
int16_t CxImageGIF::get_next_code(CxFile* file)
{
	int16_t i, x;
	uint32_t ret;

	if (nbits_left == 0)
	{
		if (navail_bytes <= 0)
		{
			/* Out of bytes in current block, so read next block */
			pbytes = byte_buff;
			if ((navail_bytes = static_cast<int16_t>(get_byte(file))) < 0)
				return (navail_bytes);
			if (navail_bytes)
			{
				for (i = 0; i < navail_bytes; ++i)
				{
					if ((x = static_cast<int16_t>(get_byte(file))) < 0) return (x);
					byte_buff[i] = static_cast<uint8_t>(x);
				}
			}
		}
		b1 = *pbytes++;
		nbits_left = 8;
		--navail_bytes;
	}

	if (navail_bytes < 0) return ending; // prevent deadlocks (thanks to Mike Melnikov)

	ret = b1 >> (8 - nbits_left);
	while (curr_size > nbits_left)
	{
		if (navail_bytes <= 0)
		{
			/* Out of bytes in current block, so read next block*/
			pbytes = byte_buff;
			if ((navail_bytes = static_cast<int16_t>(get_byte(file))) < 0)
				return (navail_bytes);
			if (navail_bytes)
			{
				for (i = 0; i < navail_bytes; ++i)
				{
					if ((x = static_cast<int16_t>(get_byte(file))) < 0) return (x);
					byte_buff[i] = static_cast<uint8_t>(x);
				}
			}
		}
		b1 = *pbytes++;
		ret |= b1 << nbits_left;
		nbits_left += 8;
		--navail_bytes;
	}
	nbits_left = static_cast<int16_t>(nbits_left - curr_size);
	ret &= code_mask[curr_size];
	return static_cast<int16_t>(ret);
}

////////////////////////////////////////////////////////////////////////////////

/* int16_t decoder(linewidth)
 *    int16_t linewidth;               * Pixels per line of image *
 *
 * - This function decodes an LZW image, according to the method used
 * in the GIF spec.  Every *linewidth* "characters" (ie. pixels) decoded
 * will generate a call to out_line(), which is a user specific function
 * to display a line of pixels.  The function gets it's codes from
 * get_next_code() which is responsible for reading blocks of data and
 * seperating them into the proper size codes.  Finally, get_byte() is
 * the global routine to read the next uint8_t from the GIF file.
 *
 * It is generally a good idea to have linewidth correspond to the actual
 * width of a line (as specified in the Image header) to make your own
 * code a bit simpler, but it isn't absolutely necessary.
 *
 * Returns: 0 if successful, else negative.  (See ERRS.H)
 *
 */
/* bad_code_count is incremented each time an out of range code is read.
 * When this value is non-zero after a decode, your GIF file is probably
 * corrupt in some way...
 */
int16_t CxImageGIF::decoder(CxFile* file, CImageIterator* iter, int16_t linewidth, int32_t& bad_code_count)
{
	uint8_t *sp, *bufptr;
	uint8_t* buf;
	int16_t code, fc, oc, bufcnt;
	int16_t c, size, ret;

	if (linewidth <= 0)
		return BAD_LINE_WIDTH;

	/* Initialize for decoding a new image... */
	bad_code_count = 0;
	if ((size = static_cast<int16_t>(get_byte(file))) < 0) return (size);
	if (size < 2 || 9 < size) return (BAD_CODE_SIZE);
	// out_line = outline;
	init_exp(size);
	//printf("L %d %x\n",linewidth,size);

	/* Initialize in case they forgot to put in a clear code.
	 * (This shouldn't happen, but we'll try and decode it anyway...)
	 */
	oc = fc = 0;

	/* Allocate space for the decode buffer */
	if ((buf = new uint8_t[linewidth + 1]) == nullptr) return (OUT_OF_MEMORY);

	/* Set up the stack pointer and decode buffer pointer */
	sp = stack;
	bufptr = buf;
	bufcnt = linewidth;

	/* This is the main loop.  For each code we get we pass through the
	 * linked list of prefix codes, pushing the corresponding "character" for
	 * each code onto the stack.  When the list reaches a single "character"
	 * we push that on the stack too, and then start unstacking each
	 * character for output in the correct order.  Special handling is
	 * included for the clear code, and the whole thing ends when we get
	 * an ending code.
	 */
	while ((c = get_next_code(file)) != ending)
	{
		/* If we had a file error, return without completing the decode*/
		if (c < 0)
		{
			delete [] buf;
			return (0);
		}
		/* If the code is a clear code, reinitialize all necessary items.*/
		if (c == clear)
		{
			curr_size = static_cast<int16_t>(size + 1);
			slot = newcodes;
			top_slot = static_cast<int16_t>(1 << curr_size);

			/* Continue reading codes until we get a non-clear code
			* (Another unlikely, but possible case...)
			*/
			while ((c = get_next_code(file)) == clear);

			/* If we get an ending code immediately after a clear code
			* (Yet another unlikely case), then break out of the loop.
			*/
			if (c == ending) break;

			/* Finally, if the code is beyond the range of already set codes,
			* (This one had better NOT happen...  I have no idea what will
			* result from this, but I doubt it will look good...) then set it
			* to color zero.
			*/
			if (c >= slot) c = 0;
			oc = fc = c;

			/* And let us not forget to put the char into the buffer... And
			* if, on the off chance, we were exactly one pixel from the end
			* of the line, we have to send the buffer to the out_line()
			* routine...
			*/
			*bufptr++ = static_cast<uint8_t>(c);
			if (--bufcnt == 0)
			{
				if (iter)
				{
					if ((ret = static_cast<int16_t>(out_line(iter, buf, linewidth))) < 0)
					{
						delete [] buf;
						return (ret);
					}
				}
				bufptr = buf;
				bufcnt = linewidth;
			}
		}
		else
		{
			/* In this case, it's not a clear code or an ending code, so
			* it must be a code code...  So we can now decode the code into
			* a stack of character codes. (Clear as mud, right?)
			*/
			code = c;

			/* Here we go again with one of those off chances...  If, on the
			* off chance, the code we got is beyond the range of those already
			* set up (Another thing which had better NOT happen...) we trick
			* the decoder into thinking it actually got the last code read.
			* (Hmmn... I'm not sure why this works...  But it does...)
			*/
			if (code >= slot && sp < (stack + MAX_CODES - 1))
			{
				if (code > slot)
					++bad_code_count;
				code = oc;
				*sp++ = static_cast<uint8_t>(fc);
			}

			/* Here we scan back along the linked list of prefixes, pushing
			* helpless characters (ie. suffixes) onto the stack as we do so.
			*/
			while (code >= newcodes && sp < (stack + MAX_CODES - 1))
			{
				*sp++ = suffix[code];
				code = prefix[code];
			}

			/* Push the last character on the stack, and set up the new
			* prefix and suffix, and if the required slot number is greater
			* than that allowed by the current bit size, increase the bit
			* size.  (NOTE - If we are all full, we *don't* save the new
			* suffix and prefix...  I'm not certain if this is correct...
			* it might be more proper to overwrite the last code...
			*/
			*sp++ = static_cast<uint8_t>(code);
			if (slot < top_slot)
			{
				suffix[slot] = static_cast<uint8_t>(fc = static_cast<uint8_t>(code));
				prefix[slot++] = oc;
				oc = c;
			}
			if (slot >= top_slot)
			{
				if (curr_size < 12)
				{
					top_slot <<= 1;
					++curr_size;
				}
			}

			/* Now that we've pushed the decoded string (in reverse order)
			* onto the stack, lets pop it off and put it into our decode
			* buffer...  And when the decode buffer is full, write another
			* line...
			*/
			while (sp > stack)
			{
				*bufptr++ = *(--sp);
				if (--bufcnt == 0)
				{
					if (iter)
					{
						if ((ret = static_cast<int16_t>(out_line(iter, buf, linewidth))) < 0)
						{
							delete [] buf;
							return (ret);
						}
					}
					bufptr = buf;
					bufcnt = linewidth;
				}
			}
		}
	}
	ret = 0;
	if (bufcnt != linewidth && iter)
		ret = static_cast<int16_t>(out_line(iter, buf, (linewidth - bufcnt)));
	delete [] buf;
	return (ret);
}

////////////////////////////////////////////////////////////////////////////////
int32_t CxImageGIF::get_num_frames(CxFile* fp, struct_TabCol* TabColSrc, struct_dscgif* dscgif)
{
	struct_image image;

	int32_t pos = fp->Tell();
	int32_t nframes = 0;

	struct_TabCol TempTabCol;
	memcpy(&TempTabCol, TabColSrc, sizeof(struct_TabCol));

	char ch;
	bool bPreviousWasNull = true;

	for (BOOL bContinue = TRUE; bContinue;)
	{
		if (fp->Read(&ch, sizeof(ch), 1) != 1) { break; }

		if (bPreviousWasNull || ch == 0)
		{
			switch (ch)
			{
			case '!': // extension
				{
					DecodeExtension(fp);
					break;
				}
			case ',': // image
				{
					assert(sizeof(image) == 9);
					//log << "Image header" << endl;
					fp->Read(&image, sizeof(image), 1);

					//avoid byte order problems with Solaris <candan> <AMSN>
					image.l = m_ntohs(image.l);
					image.t = m_ntohs(image.t);
					image.w = m_ntohs(image.w);
					image.h = m_ntohs(image.h);

					// in case of images with empty screen descriptor, give a last chance
					if (dscgif->scrwidth == 0 && dscgif->scrheight == 0)
					{
						dscgif->scrwidth = image.w;
						dscgif->scrheight = image.h;
					}

					if (((image.l + image.w) > dscgif->scrwidth) || ((image.t + image.h) > dscgif->scrheight))
						break;

					nframes++;

					// Local colour map?
					if (image.pf & 0x80)
					{
						TempTabCol.sogct = static_cast<int16_t>(1 << ((image.pf & 0x07) + 1));
						assert(3 == sizeof(struct rgb_color));
						fp->Read(TempTabCol.paleta, sizeof(struct rgb_color) * TempTabCol.sogct, 1);
						//log << "Local colour map" << endl;
					}

					int32_t badcode = 0;
					ibf = GIFBUFTAM + 1;

					interlaced = image.pf & 0x40;
					iheight = image.h;
					istep = 8;
					iypos = 0;
					ipass = 0;

					int32_t pos_start = fp->Tell();

					//if (interlaced) log << "Interlaced" << endl;
					decoder(fp, nullptr, image.w, badcode);

					if (badcode)
					{
						seek_next_image(fp, pos_start);
					}
					else
					{
						fp->Seek(-(ibfmax - ibf - 1), SEEK_CUR);
					}

					break;
				}
			case ';': //terminator
				bContinue = false;
				break;
			default:
				bPreviousWasNull = (ch == 0);
				break;
			}
		}
	}

	fp->Seek(pos,SEEK_SET);
	return nframes;
}

////////////////////////////////////////////////////////////////////////////////
int32_t CxImageGIF::seek_next_image(CxFile* fp, int32_t position)
{
	fp->Seek(position, SEEK_SET);
	char ch1, ch2;
	ch1 = ch2 = 0;
	while (fp->Read(&ch2, sizeof(char), 1) > 0)
	{
		if (ch1 == 0 && ch2 == ',')
		{
			fp->Seek(-1,SEEK_CUR);
			return fp->Tell();
		}
		ch1 = ch2;
	}
	return -1;
}
#endif //CXIMAGE_SUPPORT_DECODE
////////////////////////////////////////////////////////////////////////////////
void CxImageGIF::SetLoops(int32_t loops)
{
	m_loops = loops;
}

////////////////////////////////////////////////////////////////////////////////
int32_t CxImageGIF::GetLoops()
{
	return m_loops;
}

////////////////////////////////////////////////////////////////////////////////
void CxImageGIF::SetComment(const char* sz_comment_in)
{
	if (sz_comment_in) strncpy(m_comment, sz_comment_in, 255);
}

////////////////////////////////////////////////////////////////////////////////
void CxImageGIF::GetComment(char* sz_comment_out)
{
	if (sz_comment_out) strncpy(sz_comment_out, m_comment, 255);
}

////////////////////////////////////////////////////////////////////////////////
void CxImageGIF::GifMix(CxImage& imgsrc2, struct_image& imgdesc)
{
	int32_t ymin = max((int32_t)0, (int32_t)(GetHeight()-imgdesc.t - imgdesc.h));
	int32_t ymax = GetHeight() - imgdesc.t;
	int32_t xmin = imgdesc.l;
	int32_t xmax = min((int32_t)GetWidth(), (int32_t)(imgdesc.l + imgdesc.w));

	int32_t ibg2 = imgsrc2.GetTransIndex();
	uint8_t i2;

	for (int32_t y = ymin; y < ymax; y++)
	{
		for (int32_t x = xmin; x < xmax; x++)
		{
			i2 = imgsrc2.GetPixelIndex(x - xmin, y - ymin);
			if (i2 != ibg2) SetPixelIndex(x, y, i2);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
/*-----------------------------------------------------------------------
 *
 * miGIF Compression - mouse and ivo's GIF-compatible compression
 *
 *          -run length encoding compression routines-
 *
 * Copyright (C) 1998 Hutchison Avenue Software Corporation
 *               http://www.hasc.com
 *               info@hasc.com
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  This software is provided "AS IS." The Hutchison Avenue 
 * Software Corporation disclaims all warranties, either express or implied, 
 * including but not limited to implied warranties of merchantability and 
 * fitness for a particular purpose, with respect to this code and accompanying
 * documentation. 
 * 
 * The miGIF compression routines do not, strictly speaking, generate files 
 * conforming to the GIF spec, since the image data is not LZW-compressed 
 * (this is the point: in order to avoid transgression of the Unisys patent 
 * on the LZW algorithm.)  However, miGIF generates data streams that any 
 * reasonably sane LZW decompresser will decompress to what we want.
 *
 * miGIF compression uses run length encoding. It compresses horizontal runs 
 * of pixels of the same color. This type of compression gives good results
 * on images with many runs, for example images with lines, text and solid 
 * shapes on a solid-colored background. It gives little or no compression 
 * on images with few runs, for example digital or scanned photos.
 *
 *                               der Mouse
 *                      mouse@rodents.montreal.qc.ca
 *            7D C8 61 52 5D E7 2D 39  4E F1 31 3E E8 B3 27 4B
 *
 *                             ivo@hasc.com
 *
 * The Graphics Interchange Format(c) is the Copyright property of
 * CompuServe Incorporated.  GIF(sm) is a Service Mark property of
 * CompuServe Incorporated.
 *
 */
////////////////////////////////////////////////////////////////////////////////
void CxImageGIF::rle_clear(struct_RLE* rle)
{
	rle->out_bits = rle->out_bits_init;
	rle->out_bump = rle->out_bump_init;
	rle->out_clear = rle->out_clear_init;
	rle->out_count = 0;
	rle->rl_table_max = 0;
	rle->just_cleared = 1;
}

////////////////////////////////////////////////////////////////////////////////
void CxImageGIF::rle_flush(struct_RLE* rle)
{
	if (rle->rl_count == 1)
	{
		rle_output_plain(rle->rl_pixel, rle);
		rle->rl_count = 0;
		return;
	}
	if (rle->just_cleared)
	{
		rle_flush_fromclear(rle->rl_count, rle);
	}
	else if ((rle->rl_table_max < 2) || (rle->rl_table_pixel != rle->rl_pixel))
	{
		rle_flush_clearorrep(rle->rl_count, rle);
	}
	else
	{
		rle_flush_withtable(rle->rl_count, rle);
	}
	rle->rl_count = 0;
}

////////////////////////////////////////////////////////////////////////////////
void CxImageGIF::rle_output_plain(int32_t c, struct_RLE* rle)
{
	rle->just_cleared = 0;
	rle_output(c, rle);
	rle->out_count++;
	if (rle->out_count >= rle->out_bump)
	{
		rle->out_bits ++;
		rle->out_bump += 1 << (rle->out_bits - 1);
	}
	if (rle->out_count >= rle->out_clear)
	{
		rle_output(rle->code_clear, rle);
		rle_clear(rle);
	}
}

////////////////////////////////////////////////////////////////////////////////
void CxImageGIF::rle_flush_fromclear(int32_t count, struct_RLE* rle)
{
	int32_t n;

	rle->out_clear = rle->max_ocodes;
	rle->rl_table_pixel = rle->rl_pixel;
	n = 1;
	while (count > 0)
	{
		if (n == 1)
		{
			rle->rl_table_max = 1;
			rle_output_plain(rle->rl_pixel, rle);
			count --;
		}
		else if (count >= n)
		{
			rle->rl_table_max = n;
			rle_output_plain(rle->rl_basecode + n - 2, rle);
			count -= n;
		}
		else if (count == 1)
		{
			rle->rl_table_max ++;
			rle_output_plain(rle->rl_pixel, rle);
			count = 0;
		}
		else
		{
			rle->rl_table_max ++;
			rle_output_plain(rle->rl_basecode + count - 2, rle);
			count = 0;
		}
		if (rle->out_count == 0) n = 1;
		else n ++;
	}
	rle_reset_out_clear(rle);
}

////////////////////////////////////////////////////////////////////////////////
void CxImageGIF::rle_reset_out_clear(struct_RLE* rle)
{
	rle->out_clear = rle->out_clear_init;
	if (rle->out_count >= rle->out_clear)
	{
		rle_output(rle->code_clear, rle);
		rle_clear(rle);
	}
}

////////////////////////////////////////////////////////////////////////////////
void CxImageGIF::rle_flush_withtable(int32_t count, struct_RLE* rle)
{
	int32_t repmax;
	int32_t repleft;
	int32_t leftover;

	repmax = count / rle->rl_table_max;
	leftover = count % rle->rl_table_max;
	repleft = (leftover ? 1 : 0);
	if (rle->out_count + repmax + repleft > rle->max_ocodes)
	{
		repmax = rle->max_ocodes - rle->out_count;
		leftover = count - (repmax * rle->rl_table_max);
		repleft = 1 + rle_compute_triangle_count(leftover, rle->max_ocodes);
	}
	if (1 + rle_compute_triangle_count(count, rle->max_ocodes) < static_cast<uint32_t>(repmax + repleft))
	{
		rle_output(rle->code_clear, rle);
		rle_clear(rle);
		rle_flush_fromclear(count, rle);
		return;
	}
	rle->out_clear = rle->max_ocodes;
	for (; repmax > 0; repmax--) rle_output_plain(rle->rl_basecode + rle->rl_table_max - 2, rle);
	if (leftover)
	{
		if (rle->just_cleared)
		{
			rle_flush_fromclear(leftover, rle);
		}
		else if (leftover == 1)
		{
			rle_output_plain(rle->rl_pixel, rle);
		}
		else
		{
			rle_output_plain(rle->rl_basecode + leftover - 2, rle);
		}
	}
	rle_reset_out_clear(rle);
}

////////////////////////////////////////////////////////////////////////////////
uint32_t CxImageGIF::rle_compute_triangle_count(uint32_t count, uint32_t nrepcodes)
{
	uint32_t perrep;
	uint32_t cost;

	cost = 0;
	perrep = (nrepcodes * (nrepcodes + 1)) / 2;
	while (count >= perrep)
	{
		cost += nrepcodes;
		count -= perrep;
	}
	if (count > 0)
	{
		uint32_t n;
		n = rle_isqrt(count);
		while ((n * (n + 1)) >= 2 * count) n --;
		while ((n * (n + 1)) < 2 * count) n ++;
		cost += n;
	}
	return (cost);
}

////////////////////////////////////////////////////////////////////////////////
uint32_t CxImageGIF::rle_isqrt(uint32_t x)
{
	uint32_t r;
	uint32_t v;

	if (x < 2) return (x);
	for (v = x, r = 1; v; v >>= 2, r <<= 1);
	for (;;)
	{
		v = ((x / r) + r) / 2;
		if ((v == r) || (v == r + 1)) return (r);
		r = v;
	}
}

////////////////////////////////////////////////////////////////////////////////
void CxImageGIF::rle_flush_clearorrep(int32_t count, struct_RLE* rle)
{
	int32_t withclr;
	withclr = 1 + rle_compute_triangle_count(count, rle->max_ocodes);
	if (withclr < count)
	{
		rle_output(rle->code_clear, rle);
		rle_clear(rle);
		rle_flush_fromclear(count, rle);
	}
	else
	{
		for (; count > 0; count--) rle_output_plain(rle->rl_pixel, rle);
	}
}

////////////////////////////////////////////////////////////////////////////////
void CxImageGIF::rle_write_block(struct_RLE* rle)
{
	g_outfile->PutC(static_cast<uint8_t>(rle->oblen));
	g_outfile->Write(rle->oblock, 1, rle->oblen);
	rle->oblen = 0;
}

////////////////////////////////////////////////////////////////////////////////
void CxImageGIF::rle_block_out(uint8_t c, struct_RLE* rle)
{
	rle->oblock[rle->oblen++] = c;
	if (rle->oblen >= 255) rle_write_block(rle);
}

////////////////////////////////////////////////////////////////////////////////
void CxImageGIF::rle_block_flush(struct_RLE* rle)
{
	if (rle->oblen > 0) rle_write_block(rle);
}

////////////////////////////////////////////////////////////////////////////////
void CxImageGIF::rle_output(int32_t val, struct_RLE* rle)
{
	rle->obuf |= val << rle->obits;
	rle->obits += rle->out_bits;
	while (rle->obits >= 8)
	{
		rle_block_out(static_cast<uint8_t>(rle->obuf & 0xff), rle);
		rle->obuf >>= 8;
		rle->obits -= 8;
	}
}

////////////////////////////////////////////////////////////////////////////////
void CxImageGIF::rle_output_flush(struct_RLE* rle)
{
	if (rle->obits > 0) rle_block_out(static_cast<uint8_t>(rle->obuf), rle);
	rle_block_flush(rle);
}

////////////////////////////////////////////////////////////////////////////////
void CxImageGIF::compressRLE(int32_t init_bits, CxFile* outfile)
{
	g_init_bits = init_bits;
	g_outfile = outfile;

	struct_RLE rle;
	rle.code_clear = 1 << (init_bits - 1);
	rle.code_eof = rle.code_clear + 1;
	rle.rl_basecode = rle.code_eof + 1;
	rle.out_bump_init = (1 << (init_bits - 1)) - 1;
	rle.out_clear_init = (init_bits <= 3) ? 9 : (rle.out_bump_init - 1);
	rle.out_bits_init = init_bits;
	rle.max_ocodes = (1 << MAXBITSCODES) - ((1 << (rle.out_bits_init - 1)) + 3);
	rle.rl_count = 0;
	rle_clear(&rle);
	rle.obuf = 0;
	rle.obits = 0;
	rle.oblen = 0;

	rle_output(rle.code_clear, &rle);

	int32_t c;
	for (;;)
	{
		c = GifNextPixel();
		if ((rle.rl_count > 0) && (c != rle.rl_pixel)) rle_flush(&rle);
		if (c == EOF) break;
		if (rle.rl_pixel == c)
		{
			rle.rl_count++;
		}
		else
		{
			rle.rl_pixel = c;
			rle.rl_count = 1;
		}
	}
	rle_output(rle.code_eof, &rle);
	rle_output_flush(&rle);
}

////////////////////////////////////////////////////////////////////////////////
#endif // CXIMAGE_SUPPORT_GIF
