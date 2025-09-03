#ifndef AGAPE_ASSETS_SAUCE_H
#define AGAPE_ASSETS_SAUCE_H

// SAUCE v00.5
// Credz to Olivier "Tasmaniac" Reubens and the ACiD crew.

#include "String.h"

namespace Agape
{

class Asset;
class Clock;

namespace Assets
{

class SAUCE
{
public:
    enum DataTypes
    {
        dtNone,
        dtCharacter,
        dtBitmap,
        dtVector,
        dtAudio,
        dtBinaryText,
        dtXBin,
        dtArchive,
        dtExecutable
    };

    enum NoneFileTypes
    {
        ftnNone
    };

    enum CharacterFileTypes
    {
        ftcASCII,
        ftcANSI,
        ftcANSIMation,
        ftcRIPScript,
        ftcPCBoard,
        ftcAvatar,
        ftcHTML,
        ftcSource,
        ftcTundraDraw
    };

    enum BitmapFileTypes
    {
        ftbGIF,
        ftbPCX,
        ftbLBMIFF,
        ftbTGA,
        ftbFLI,
        ftbFLC,
        ftbBMP,
        flbGL,
        ftbDL,
        ftbWPG,
        ftbPNG,
        ftbJPG,
        ftbMPG,
        ftbAVI
    };

    enum VectorFileTypes
    {
        ftvDXF,
        ftvFWG,
        ftvWPG,
        ftv3DS
    };

    enum AudioFileTypes
    {
        ftaMOD,
        fta669,
        ftaSTM,
        ftaS3M,
        ftaMTM,
        ftaFAR,
        ftaULT,
        ftaAMF,
        ftaDMF,
        ftaOKT,
        ftaROL,
        ftaCMF,
        ftaMID,
        ftaSADT,
        ftaVOC,
        ftaWAV,
        ftaSMP8,
        ftaSMP8S,
        ftaSMP16,
        ftaSMP16S,
        ftaPATCH8,
        ftaPATCH16,
        ftaXM,
        ftaHSC,
        ftaIT
    };

    enum ArchiveFileTypes
    {
        ftrZIP,
        ftrARJ,
        ftrLZH,
        ftrARC,
        ftrTAR,
        ftrZOO,
        ftrRAR,
        ftrUC2,
        ftrPAK,
        ftrSQZ
    };

    enum ANSIFlags
    {
        flgiCEColours = 1,
        flgLetterSpacing8px = 2,
        flgLetterSpacing9px = 4,
        flgAspectRatioLegacy = 8,
        flgAspectRatioModern = 16
    };

    static bool extractSAUCE( Asset& asset, SAUCE& sauce );

    SAUCE();
    SAUCE( const SAUCE& other );
    ~SAUCE();

    String title() const;
    String author() const;
    String group() const;
    String date() const;
    int fileSize() const;
    int dataType() const;
    int fileType() const;
    int tInfo1() const;
    int tInfo2() const;
    int tInfo3() const;
    int tInfo4() const;
    int tFlags() const;
    String tInfoS() const;
    String comments() const;

    void setTitle( const String& title );
    void setAuthor( const String& author );
    void setGroup( const String& group );
    void setFileSize( int fileSize );
    void setDataType( int dataType );
    void setFileType( int fileType );
    void setTInfo1( int tInfo1 );
    void setTInfo2( int tInfo2 );
    void setTInfo3( int tInfo3 );
    void setTInfo4( int tInfo4 );
    void setTFlags( int tFlags );
    void setTInfoS( const String& tInfoS );
    void setComments( const String& comments );

    void setDateFromClock( Clock& clock );

    bool writeSAUCE( Asset& asset, int offset );

    SAUCE& operator=( const SAUCE& other );

private:

    char m_id[5];
    char m_version[2];
    char m_title[35];
    char m_author[20];
    char m_group[20];
    char m_date[8];
    unsigned int m_fileSize;
    unsigned char m_dataType;
    unsigned char m_fileType;
    unsigned short m_tInfo1;
    unsigned short m_tInfo2;
    unsigned short m_tInfo3;
    unsigned short m_tInfo4;
    unsigned char m_numCommentLines;
    unsigned char m_tFlags;
    char m_tInfoS[22];

    char* m_comments;
};



} // namespace Assets

} // namespace Agape

#endif // AGAPE_ASSETS_SAUCE_H
