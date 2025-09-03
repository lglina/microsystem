#ifndef AGAPE_WORLD_SCENE_ITEM_H
#define AGAPE_WORLD_SCENE_ITEM_H

#include "EncryptableDecryptable.h"
#include "String.h"

namespace Agape
{

class Clock;
class Encryptor;
class Value;

namespace World
{

class SceneItem : public EncryptableDecryptable
{
public:
    enum Flags
    {
        none = 0,
        linkedProgram = 1,
        encrypted = 2,
        linkedText = 4
    };

    SceneItem();
    SceneItem( const String& assetName,
               int row,
               int col,
               int height,
               int width,
               const String& owner,
               const String& action,
               Clock& clock );
    SceneItem( const SceneItem& other );

    ~SceneItem();

    String assetName() const;
    int row() const;
    int col() const;
    int height() const;
    int width() const;
    String dateTime() const;
    String owner() const;
    String action() const;
    String snowflake() const;
    String modificationSnowflake() const;
    char flags() const;

    void setAssetName( const String& assetName );
    void setRow( int row );
    void setCol( int col );
    void setDimensions( int height, int width );
    void setAction( const String& action );
    void setSnowflake( const String& snowflake );
    void setFlags( char flags );

    void touch();

    static int maxAssetNameLength();
    static int maxActionLength();

    void toValue( Value& value ) const;
    static SceneItem fromValue( const Value& value );

    virtual bool encrypt( Encryptor& encryptor );
    virtual bool decrypt( Encryptor& encryptor );

    SceneItem& operator=( const SceneItem& other );

    bool operator==( const SceneItem& other ) const;
    bool operator!=( const SceneItem& other ) const;

    bool operator<( const SceneItem& other ) const;

private:
    char m_assetName[48];
    char m_row;
    char m_col;
    char m_height;
    char m_width;
    char m_dateTime[14];
    char m_owner[16];
    char* m_action;
    char m_snowflake[16];
    char m_modificationSnowflake[16];
    char m_flags;
};

} // namespace World

} // namespace Agape

#endif // AGAPE_WORLD_SCENE_ITEM_H
