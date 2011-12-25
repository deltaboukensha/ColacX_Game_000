#include "Texture.h"

Texture::Texture(const char* textureFilepath){
	loadTGA( textureFilepath );
}

void Texture::loadTGA(const char* textureFilepath){
    tgaImageFile tgaImage;
    tgaImage.load( (char*)textureFilepath );

    this->textureID = 0;
    glGenTextures( 1, &this->textureID );

    glBindTexture( GL_TEXTURE_2D, this->textureID );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glTexImage2D( GL_TEXTURE_2D, 0, tgaImage.m_texFormat, tgaImage.m_nImageWidth, tgaImage.m_nImageHeight, 0, tgaImage.m_texFormat, GL_UNSIGNED_BYTE, tgaImage.m_nImageData );
}

Texture::~Texture(){
    glDeleteTextures(1,&this->textureID);
}

void Texture::bind(){
    glBindTexture(GL_TEXTURE_2D, this->textureID);
}

tgaImageFile::~tgaImageFile( void )
{
    if( m_nImageData != NULL )
    {
        free( m_nImageData );
        m_nImageData = NULL;
    }
}

bool tgaImageFile::checkSize( int x )
{
    // Make sure its a power of 2.
    if( x == 2     || x == 4   || 
        x == 8     || x == 16  || 
        x == 32  || x == 64  ||
        x == 128 || x == 256 || 
        x == 512 )
        return true;

    return false;
}

int tgaImageFile::returnError( FILE *s, int error )
{
    // Called when there is an error loading the .tga texture file.
    fclose( s );
    return error;
}

unsigned char *tgaImageFile::getRGBA( FILE *s, int size )
{
    // Read in RGBA data for a 32bit image. 
    unsigned char *rgba;
    unsigned char temp;
    int bread;
    int i;

    rgba = (unsigned char *)malloc( size * 4 );

    if( rgba == NULL )
        return 0;

    bread = fread( rgba, sizeof (unsigned char), size * 4, s ); 

    // TGA is stored in BGRA, make it RGBA  
    if( bread != size * 4 )
    {
        free( rgba );
        return 0;
    }

    for( i = 0; i < size * 4; i += 4 )
    {
        temp = rgba[i];
        rgba[i] = rgba[i + 2];
        rgba[i + 2] = temp;
    }

    m_texFormat = GL_RGBA;
    return rgba;
}

unsigned char *tgaImageFile::getRGB( FILE *s, int size )
{
    // Read in RGB data for a 24bit image. 
    unsigned char *rgb;
    unsigned char temp;
    int bread;
    int i;

    rgb = (unsigned char*)malloc( size * 3 );

    if( rgb == NULL )
        return 0;

    bread = fread( rgb, sizeof (unsigned char), size * 3, s );

    if(bread != size * 3)
    {
        free( rgb );
        return 0;
    }

    // TGA is stored in BGR, make it RGB  
    for( i = 0; i < size * 3; i += 3 )
    {
        temp = rgb[i];
        rgb[i] = rgb[i + 2];
        rgb[i + 2] = temp;
    }

    m_texFormat = GL_RGB;

    return rgb;
}

unsigned char *tgaImageFile::getGray( FILE *s, int size )
{
    // Gets the grayscale image data.  Used as an alpha channel.
    unsigned char *grayData;
    int bread;

    grayData = (unsigned char*)malloc( size );

    if( grayData == NULL )
        return 0;

    bread = fread( grayData, sizeof (unsigned char), size, s );

    if( bread != size )
    {
        free( grayData );
        return 0;
    }

    m_texFormat = GL_ALPHA;

    return grayData;
}

tgaImageFile::TGALoadError tgaImageFile::load( char *name )
{
    // Loads up a targa file. Supported types are 8,24 and 32 
    // uncompressed images.
    unsigned char type[4];
    unsigned char info[7];
    FILE *s = 0;
    int size = 0;

    if( fopen_s(&s,name, "rb") != 0 ){
        return TGA_FILE_NOT_FOUND;
    }

    fread( &type, sizeof (char), 3, s );   // Read in colormap info and image type, byte 0 ignored
    fseek( s, 12, SEEK_SET);               // Seek past the header and useless info
    fread( &info, sizeof (char), 6, s );

    if( type[1] != 0 || (type[2] != 2 && type[2] != 3) )
        returnError( s, TGA_BAD_IMAGE_TYPE );

    m_nImageWidth  = info[0] + info[1] * 256; 
    m_nImageHeight = info[2] + info[3] * 256;
    m_nImageBits   = info[4]; 

    size = m_nImageWidth * m_nImageHeight;

    // Make sure dimension is a power of 2  
    if( !checkSize(m_nImageWidth) || !checkSize(m_nImageHeight))
        returnError( s, TGA_BAD_DIMENSION);

    // Make sure we are loading a supported type  
    if( m_nImageBits != 32 && m_nImageBits != 24 && m_nImageBits != 8 )
        returnError( s, TGA_BAD_BITS );

    if( m_nImageBits == 32 )
        m_nImageData = getRGBA( s, size );
    else if( m_nImageBits == 24 )
        m_nImageData = getRGB( s, size );    
    else if( m_nImageBits == 8 )
        m_nImageData = getGray( s, size );

    // No image data 
    if( m_nImageData == NULL )
        returnError( s, TGA_BAD_DATA );

    fclose( s );

    return TGA_NO_ERROR;
}