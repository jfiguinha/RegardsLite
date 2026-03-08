#include "header.h"
#include "2PassScale.h"


void C2PassScale::Execute(const cv::Mat& in, cv::Mat& Out)
{
    if (in.empty() || Out.empty() || in.rows <= 0 || in.cols <= 0 || Out.rows <= 0 || Out.cols <= 0)
    {
        throw std::invalid_argument("Les matrices d'entrée ou de sortie sont invalides.");
    }
	Scale((uint32_t*)in.data, in.cols, in.rows, (uint32_t*)Out.data, Out.cols, Out.rows);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////////////////
void C2PassScale::Scale(uint32_t *pOrigImage, int uOrigWidth, int uOrigHeight, uint32_t *pDstImage, int uNewWidth, int uNewHeight)
{
    // Scale source image horizontally into temporary image
    uint32_t *pTemp = new uint32_t [uNewWidth * uOrigHeight];
    HorizScale (pOrigImage, 
                uOrigWidth,
                uOrigHeight,
                pTemp,
                uNewWidth,
                uOrigHeight);

    // Scale temporary image vertically into result image    
    VertScale ( pTemp,
                uNewWidth,
                uOrigHeight,
                pDstImage,
                uNewWidth,
                uNewHeight);
    delete [] pTemp;

}

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////////////////
C2PassScale::LineContribType * C2PassScale::AllocContributions (int uLineLength, int uWindowSize)
{
    LineContribType *res = new LineContribType; 
        // Init structure header 
    res->WindowSize = uWindowSize; 
    res->LineLength = uLineLength; 
        // Allocate list of contributions 
    res->ContribRow = new ContributionType[uLineLength];
    for (int u = 0 ; u < uLineLength ; u++) 
    {
        // Allocate contributions for every pixel
        res->ContribRow[u].Weights = new double[uWindowSize];
    }
    return res; 
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////////////////
void C2PassScale::FreeContributions (LineContribType * p)
{
    for (int u = 0; u < p->LineLength; u++) 
    {
        // Free contribs for every pixel
        delete [] p->ContribRow[u].Weights;
    }
    delete [] p->ContribRow;    // Free list of pixels contribs
    delete p;                   // Free contribs header
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////////////////
double C2PassScale::Filter(const double &dVal)
{
    double m_dLocalVal = fabs(dVal); 
    return (m_dLocalVal < m_dWidth ? m_dWidth - m_dLocalVal : 0.0); 
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////////////////
C2PassScale::LineContribType * C2PassScale::CalcContributions(int uLineSize, int uSrcSize, double  dScale)
{
    double dWidth;
    double dFScale = 1.0;

    if (dScale < 1.0) 
    {    // Minification
        dWidth = m_dWidth / dScale; 
        dFScale = dScale; 
    } 
    else
    {    // Magnification
        dWidth= m_dWidth; 
    }
 
    // Window size is the number of sampled pixels
    int iWindowSize = 2 * (int)ceil(dWidth) + 1; 
 
    // Allocate a new line contributions strucutre
    LineContribType *res = AllocContributions (uLineSize, iWindowSize); 
 
    for (int u = 0; u < uLineSize; u++) 
    {   // Scan through line of contributions
        double dCenter = (double)u / dScale;   // Reverse mapping
        // Find the significant edge points that affect the pixel
        int iLeft = max (0, (int)floor (dCenter - dWidth)); 
        res->ContribRow[u].Left = iLeft; 
        int iRight = min ((int)ceil (dCenter + dWidth), int(uSrcSize) - 1); 
        res->ContribRow[u].Right = iRight;
 
        // Cut edge points to fit in filter window in case of spill-off
        if (iRight - iLeft + 1 > iWindowSize) 
        {
            if (iLeft < (int(uSrcSize) - 1 / 2)) 
            {
                iLeft++; 
            }
            else 
            {
                iRight--; 
            }
        }
        double dTotalWeight = 0.0;  // Zero sum of weights
        for (int iSrc = iLeft; iSrc <= iRight; iSrc++)
        {   // Calculate weights
            dTotalWeight += (res->ContribRow[u].Weights[iSrc-iLeft] =  
                                dFScale * Filter(dFScale * (dCenter - (double)iSrc))); 
        }
        //ASSERT (dTotalWeight >= 0.0);   // An error in the filter function can cause this 
        if (dTotalWeight > 0.0)
        {   // Normalize weight of neighbouring points
            for (int iSrc = iLeft; iSrc <= iRight; iSrc++)
            {   // Normalize point
                res->ContribRow[u].Weights[iSrc-iLeft] /= dTotalWeight; 
            }
        }
   } 
   return res; 
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////////////////
void C2PassScale::ScaleRow(uint32_t *pSrc, int uSrcWidth,uint32_t *pRes, int uResWidth,int uRow, LineContribType *Contrib)
{
    uint32_t *pSrcRow = &(pSrc[uRow * uSrcWidth]);
    uint32_t *pDstRow = &(pRes[uRow * uResWidth]);
#pragma omp parallel for
    for (int x = 0; x < uResWidth; x++) 
    {   // Loop through row
        double r = 0;
        double g = 0;
        double b = 0;
        int iLeft = Contrib->ContribRow[x].Left;    // Retrieve left boundries
        int iRight = Contrib->ContribRow[x].Right;  // Retrieve right boundries
        for (int i = iLeft; i <= iRight; i++)
        {   // Scan between boundries
            // Accumulate weighted effect of each neighboring pixel
			
            r += (BYTE)(Contrib->ContribRow[x].Weights[i-iLeft] * (double)(GetRValue(pSrcRow[i]))); 
            g += (BYTE)(Contrib->ContribRow[x].Weights[i-iLeft] * (double)(GetGValue(pSrcRow[i]))); 
            b += (BYTE)(Contrib->ContribRow[x].Weights[i-iLeft] * (double)(GetBValue(pSrcRow[i]))); 
        } 

        pDstRow[x] = RGB(r,g,b); // Place result in destination pixel
    } 
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////////////////
void C2PassScale::HorizScale(uint32_t *pSrc, int uSrcWidth,int uSrcHeight,uint32_t *pDst,int uResWidth,int uResHeight)
{
    //TRACE ("Performing horizontal scaling...\n"); 
    if (uResWidth == uSrcWidth)
    {   // No scaling required, just copy
		memcpy(pDst, pSrc, sizeof (uint32_t) * uSrcHeight * uSrcWidth);
    }
    // Allocate and calculate the contributions
    LineContribType * Contrib = CalcContributions (uResWidth, uSrcWidth, double(uResWidth) / double(uSrcWidth)); 
    for (int u = 0; u < uResHeight; u++)
    {   // Step through rows              
        ScaleRow (  pSrc, 
                    uSrcWidth,
                    pDst,
                    uResWidth,
                    u,
                    Contrib);    // Scale each row 
    }
    FreeContributions (Contrib);  // Free contributions structure
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////////////////
void C2PassScale::ScaleCol(uint32_t *pSrc, int uSrcWidth,uint32_t *pRes, int uResWidth,int uResHeight,int uCol, LineContribType *Contrib)
{
    for (int y = 0; y < uResHeight; y++) 
    {    // Loop through column
        double r = 0;
        double g = 0;
        double b = 0;
        int iLeft = Contrib->ContribRow[y].Left;    // Retrieve left boundries
        int iRight = Contrib->ContribRow[y].Right;  // Retrieve right boundries
        for (int i = iLeft; i <= iRight; i++)
        {   // Scan between boundries
            // Accumulate weighted effect of each neighboring pixel

			//A mettre dans un tableau pour Traitement SSE

            uint32_t pCurSrc = pSrc[i * uSrcWidth + uCol];
            r += BYTE(Contrib->ContribRow[y].Weights[i-iLeft] * (double)(GetRValue(pCurSrc)));
            g += BYTE(Contrib->ContribRow[y].Weights[i-iLeft] * (double)(GetGValue(pCurSrc)));
            b += BYTE(Contrib->ContribRow[y].Weights[i-iLeft] * (double)(GetBValue(pCurSrc)));
        }
        pRes[y * uResWidth + uCol] = RGB (r,g,b);   // Place result in destination pixel
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////////////////
void C2PassScale::VertScale(uint32_t *pSrc, int uSrcWidth, int uSrcHeight, uint32_t *pDst, int uResWidth, int uResHeight)
{
    if (uSrcHeight == uResHeight)
    {   // No scaling required, just copy
        memcpy(pDst, pSrc, sizeof (uint32_t) * uSrcHeight * uSrcWidth);
    }
    // Allocate and calculate the contributions
    LineContribType * Contrib = CalcContributions (uResHeight, uSrcHeight, double(uResHeight) / double(uSrcHeight)); 

#pragma omp parallel for
    for (int u = 0; u < uResWidth; u++)
    {   // Step through columns
        ScaleCol (  pSrc,
                    uSrcWidth,
                    pDst,
                    uResWidth,
                    uResHeight,
                    u,
                    Contrib);   // Scale each column
    }
    FreeContributions (Contrib);     // Free contributions structure
}
