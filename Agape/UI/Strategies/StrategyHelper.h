#ifndef AGAPE_UI_STRATEGIES_HELPER_H
#define AGAPE_UI_STRATEGIES_HELPER_H

namespace Agape
{

class String;
class Terminal;

namespace UI
{

namespace Strategies
{

class Helper
{
public:
    static void drawMenuBackground( Terminal* terminal, const String& textAssetName );
};

} // namespace Strategies

} // namespace UI

} // namespace Agape

#endif // AGAPE_UI_STRATEGIES_HELPER_H
