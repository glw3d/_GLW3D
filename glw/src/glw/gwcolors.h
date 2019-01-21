/**
Author: Mario J. Martin <dominonurbs$gmail.com>

X11 web colors. Extracted from https://en.wikipedia.org/wiki/Web_colors
And my favorite Crayola colors.

Did you know that fuchsia and magenta are the same color?
*******************************************************************************/

#ifndef _HGW_COLORS_H
#define _HGW_COLORS_H

#include <stdint.h>
#include "gwdata.h"

#define _hex2bgra_( h ) { uint8_t(h & 0x0000FF)\
    , uint8_t((h & 0x00FF00) >> 8)\
    , uint8_t((h & 0xFF0000) >> 16), uint8_t(0xFF) }

/* Essential colors */
const gwColor gwColorWhite    = _hex2bgra_( 0xFFFFFF );
const gwColor gwColorSilver   = _hex2bgra_( 0xC0C0C0 );
const gwColor gwColorGray     = _hex2bgra_( 0x808080 );
const gwColor gwColorBlack    = _hex2bgra_( 0x000000 );
const gwColor gwColorRed      = _hex2bgra_( 0xFF0000 );
const gwColor gwColorMaroon   = _hex2bgra_( 0x800000 );
const gwColor gwColorYellow   = _hex2bgra_( 0xFFFF00 );
const gwColor gwColorOlive    = _hex2bgra_( 0x808000 );
const gwColor gwColorLime     = _hex2bgra_( 0x00FF00 );
const gwColor gwColorGreen    = _hex2bgra_( 0x008000 );
const gwColor gwColorAqua     = _hex2bgra_( 0x00FFFF );
const gwColor gwColorCyan     = _hex2bgra_( 0x00FFFF );
const gwColor gwColorTeal     = _hex2bgra_( 0x008080 );
const gwColor gwColorBlue     = _hex2bgra_( 0x0000FF );
const gwColor gwColorNavy     = _hex2bgra_( 0x000080 );
const gwColor gwColorFuchsia  = _hex2bgra_( 0xFF00FF );
const gwColor gwColorMagenta  = _hex2bgra_( 0xFF00FF );
const gwColor gwColorPurple   = _hex2bgra_( 0x800080 );
const gwColor gwColorOrange    = _hex2bgra_( 0xFF8C00 );



/*** Crayola Colors ***/
const gwColor gwCrayolaAlmond              = _hex2bgra_( 0xEFDECD );
const gwColor gwCrayolaAntiqueBrass        = _hex2bgra_( 0xCD9575 );
const gwColor gwCrayolaApricot             = _hex2bgra_( 0xFDD9B5 );
const gwColor gwCrayolaAquamarine          = _hex2bgra_( 0x78DBE2 );
const gwColor gwCrayolaAsparagus           = _hex2bgra_( 0x87A96B );
const gwColor gwCrayolaAtomicTangerine     = _hex2bgra_( 0xFFA474 );

const gwColor gwCrayolaBananaMania         = _hex2bgra_( 0xFAE7B5 );
const gwColor gwCrayolaBeaver              = _hex2bgra_( 0x9F8170 );
const gwColor gwCrayolaBittersweet         = _hex2bgra_( 0xFD7C6E );
const gwColor gwCrayolaBlack               = _hex2bgra_( 0x000000 );
const gwColor gwCrayolaBlizzardBlue        = _hex2bgra_( 0xACE5EE );
const gwColor gwCrayolaBlue                = _hex2bgra_( 0x1F75FE );
const gwColor gwCrayolaBlueBell            = _hex2bgra_( 0xA2A2D0 );
const gwColor gwCrayolaBlueGray            = _hex2bgra_( 0x6699CC );
const gwColor gwCrayolaBlueGreen           = _hex2bgra_( 0x0D98BA );
const gwColor gwCrayolaBlueViolet          = _hex2bgra_( 0x7366BD );
const gwColor gwCrayolaBlush               = _hex2bgra_( 0xDE5D83 );
const gwColor gwCrayolaBrickRed            = _hex2bgra_( 0xCB4154 );
const gwColor gwCrayolaBrown               = _hex2bgra_( 0xB4674D );
const gwColor gwCrayolaBurntOrange         = _hex2bgra_( 0xFF7F49 );
const gwColor gwCrayolaBurntSienna         = _hex2bgra_( 0xEA7E5D );

const gwColor gwCrayolaCadetBlue           = _hex2bgra_( 0xB0B7C6 );
const gwColor gwCrayolaCanary              = _hex2bgra_( 0xFFFF99 );
const gwColor gwCrayolaCaribbeanGreen      = _hex2bgra_( 0x00CC99 );
const gwColor gwCrayolaCarnationPink       = _hex2bgra_( 0xFFAACC );
const gwColor gwCrayolaCerise              = _hex2bgra_( 0xDD4492 );
const gwColor gwCrayolaCerulean            = _hex2bgra_( 0x1DACD6 );
const gwColor gwCrayolaChestnut            = _hex2bgra_( 0xBC5D58 );
const gwColor gwCrayolaCopper              = _hex2bgra_( 0xDD9475 );
const gwColor gwCrayolaCornflower          = _hex2bgra_( 0x9ACEEB );
const gwColor gwCrayolaCottonCandy         = _hex2bgra_( 0xFFBCD9 );

const gwColor gwCrayolaDandelion           = _hex2bgra_( 0xFDDB6D );
const gwColor gwCrayolaDenim               = _hex2bgra_( 0x2B6CC4 );
const gwColor gwCrayolaDesertSand          = _hex2bgra_( 0xEFCDB8 );
const gwColor gwCrayolaEggplant            = _hex2bgra_( 0x6E5160 );
const gwColor gwCrayolaElectricLime        = _hex2bgra_( 0xCEFF1D );
const gwColor gwCrayolaFern                = _hex2bgra_( 0x71BC78 );
const gwColor gwCrayolaForestGreen         = _hex2bgra_( 0x6DAE81 );
const gwColor gwCrayolaFuchsia             = _hex2bgra_( 0xC364C5 );
const gwColor gwCrayolaFuzzyWuzzy          = _hex2bgra_( 0xCC6666 );

const gwColor gwCrayolaGold                = _hex2bgra_( 0xE7C697 );
const gwColor gwCrayolaGoldenrod           = _hex2bgra_( 0xFCD975 );
const gwColor gwCrayolaGrannySmithApple    = _hex2bgra_( 0xA8E4A0 );
const gwColor gwCrayolaGray                = _hex2bgra_( 0x95918C );
const gwColor gwCrayolaGreen               = _hex2bgra_( 0x1CAC78 );
const gwColor gwCrayolaGreenBlue           = _hex2bgra_( 0x1164B4 );
const gwColor gwCrayolaGreenYellow         = _hex2bgra_( 0xF0E891 );

const gwColor gwCrayolaHotMagenta          = _hex2bgra_( 0xFF1DCE );
const gwColor gwCrayolaInchworm            = _hex2bgra_( 0xB2EC5D );
const gwColor gwCrayolaIndigo              = _hex2bgra_( 0x5D76CB );
const gwColor gwCrayolaJazzberryJam        = _hex2bgra_( 0xCA3767 );
const gwColor gwCrayolaJungleGreen         = _hex2bgra_( 0x3BB08F );
const gwColor gwCrayolaLaserLemon          = _hex2bgra_( 0xFEFE22 );
const gwColor gwCrayolaLavender            = _hex2bgra_( 0xFCB4D5 );
const gwColor gwCrayolaLemonYellow         = _hex2bgra_( 0xFFF44F );

const gwColor gwCrayolaMacaroniAndCheese   = _hex2bgra_( 0xFFBD88 );
const gwColor gwCrayolaMagenta             = _hex2bgra_( 0xF664AF );
const gwColor gwCrayolaMagicMint           = _hex2bgra_( 0xAAF0D1 );
const gwColor gwCrayolaMahogany            = _hex2bgra_( 0xCD4A4C );
const gwColor gwCrayolaMaize               = _hex2bgra_( 0xEDD19C );
const gwColor gwCrayolaManatee             = _hex2bgra_( 0x979AAA );
const gwColor gwCrayolaMangoTango          = _hex2bgra_( 0xFF8243 );
const gwColor gwCrayolaMaroon              = _hex2bgra_( 0xC8385A );
const gwColor gwCrayolaMauvelous           = _hex2bgra_( 0xEF98AA );
const gwColor gwCrayolaMelon               = _hex2bgra_( 0xFDBCB4 );
const gwColor gwCrayolaMidnightBlue        = _hex2bgra_( 0x1A4876 );
const gwColor gwCrayolaMountainMeadow      = _hex2bgra_( 0x30BA8F );
const gwColor gwCrayolaMulberry            = _hex2bgra_( 0xC54B8C );

const gwColor gwCrayolaNavyBlue            = _hex2bgra_( 0x1974D2 );
const gwColor gwCrayolaNeonCarrot          = _hex2bgra_( 0xFFA343 );
const gwColor gwCrayolaOliveGreen          = _hex2bgra_( 0xBAB86C );
const gwColor gwCrayolaOrange              = _hex2bgra_( 0xFF7538 );
const gwColor gwCrayolaOrangeRed           = _hex2bgra_( 0xFF2B2B );
const gwColor gwCrayolaOrangeYellow        = _hex2bgra_( 0xF8D568 );
const gwColor gwCrayolaOrchid              = _hex2bgra_( 0xE6A8D7 );
const gwColor gwCrayolaOuterSpace          = _hex2bgra_( 0x414A4C );
const gwColor gwCrayolaOutrageousOrange    = _hex2bgra_( 0xFF6E4A );

const gwColor gwCrayolaPacificBlue         = _hex2bgra_( 0x1CA9C9 );
const gwColor gwCrayolaPeach               = _hex2bgra_( 0xFFCFAB );
const gwColor gwCrayolaPeriwinkle          = _hex2bgra_( 0xC5D0E6 );
const gwColor gwCrayolaPiggyPink           = _hex2bgra_( 0xFDDDE6 );
const gwColor gwCrayolaPineGreen           = _hex2bgra_( 0x158078 );
const gwColor gwCrayolaPinkFlamingo        = _hex2bgra_( 0xFC74FD );
const gwColor gwCrayolaPinkSherbert        = _hex2bgra_( 0xF78FA7 );
const gwColor gwCrayolaPlum                = _hex2bgra_( 0x8E4585 );
const gwColor gwCrayolaPurpleHeart         = _hex2bgra_( 0x7442C8 );
const gwColor gwCrayolaPurpleMountainsMajesty = _hex2bgra_( 0x9D81BA );
const gwColor gwCrayolaPurplePizzazz       = _hex2bgra_( 0xFE4EDA );

const gwColor gwCrayolaRadicalRed          = _hex2bgra_( 0xFF496C );
const gwColor gwCrayolaRawSienna           = _hex2bgra_( 0xD68A59 );
const gwColor gwCrayolaRawUmber            = _hex2bgra_( 0x714B23 );
const gwColor gwCrayolaRazzleDazzleRose    = _hex2bgra_( 0xFF48D0 );
const gwColor gwCrayolaRazzmatazz          = _hex2bgra_( 0xE3256B );
const gwColor gwCrayolaRed                 = _hex2bgra_( 0xEE204D );
const gwColor gwCrayolaRedOrange           = _hex2bgra_( 0xFF5349 );
const gwColor gwCrayolaRedViolet           = _hex2bgra_( 0xC0448F );
const gwColor gwCrayolaRobinsEggBlue       = _hex2bgra_( 0x1FCECB );
const gwColor gwCrayolaRoyalPurple         = _hex2bgra_( 0x7851A9 );

const gwColor gwCrayolaSalmon              = _hex2bgra_( 0xFF9BAA );
const gwColor gwCrayolaScarlet             = _hex2bgra_( 0xFC2847 );
const gwColor gwCrayolaScreaminGreen       = _hex2bgra_( 0x76FF7A );
const gwColor gwCrayolaSeaGreen            = _hex2bgra_( 0x93DFB8 );
const gwColor gwCrayolaSepia               = _hex2bgra_( 0xA5694F );
const gwColor gwCrayolaShadow              = _hex2bgra_( 0x8A795D );
const gwColor gwCrayolaShamrock            = _hex2bgra_( 0x45CEA2 );
const gwColor gwCrayolaShockingPink        = _hex2bgra_( 0xFB7EFD );
const gwColor gwCrayolaSilver              = _hex2bgra_( 0xCDC5C2 );
const gwColor gwCrayolaSkyBlue             = _hex2bgra_( 0x80DAEB );
const gwColor gwCrayolaSpringGreen         = _hex2bgra_( 0xECEABE );
const gwColor gwCrayolaSunglow             = _hex2bgra_( 0xFFCF48 );
const gwColor gwCrayolaSunsetOrange        = _hex2bgra_( 0xFD5E53 );

const gwColor gwCrayolaTan                 = _hex2bgra_( 0xFAA76C );
const gwColor gwCrayolaTealBlue            = _hex2bgra_( 0x18A7B5 );
const gwColor gwCrayolaThistle             = _hex2bgra_( 0xEBC7DF );
const gwColor gwCrayolaTickleMePink        = _hex2bgra_( 0xFC89AC );
const gwColor gwCrayolaTimberwolf          = _hex2bgra_( 0xDBD7D2 );
const gwColor gwCrayolaTropicalRainForest  = _hex2bgra_( 0x17806D );
const gwColor gwCrayolaTumbleweed          = _hex2bgra_( 0xDEAA88 );
const gwColor gwCrayolaTurquoiseBlue       = _hex2bgra_( 0x77DDE7 );

const gwColor gwCrayolaUnmellowYellow      = _hex2bgra_( 0xFFFF66 );
const gwColor gwCrayolaVioletPurple        = _hex2bgra_( 0x926EAE );
const gwColor gwCrayolaVioletBlue          = _hex2bgra_( 0x324AB2 );
const gwColor gwCrayolaVioletRed           = _hex2bgra_( 0xF75394 );
const gwColor gwCrayolaVividTangerine      = _hex2bgra_( 0xFFA089 );
const gwColor gwCrayolaVividViolet         = _hex2bgra_( 0x8F509D );

const gwColor gwCrayolaWhite               = _hex2bgra_( 0xFFFFFF );
const gwColor gwCrayolaWildBlueYonder      = _hex2bgra_( 0xA2ADD0 );
const gwColor gwCrayolaWildStrawberry      = _hex2bgra_( 0xFF43A4 );
const gwColor gwCrayolaWildWatermelon      = _hex2bgra_( 0xFC6C85 );
const gwColor gwCrayolaWisteria            = _hex2bgra_( 0xCDA4DE );
const gwColor gwCrayolaYellow              = _hex2bgra_( 0xFCE883 );
const gwColor gwCrayolaYellowGreen         = _hex2bgra_( 0xC5E384 );
const gwColor gwCrayolaYellowOrange        = _hex2bgra_( 0xFFAE42 );



/*** Web colors ***/
/* Reds */
const gwColor gwWebLightCoral   = _hex2bgra_( 0xF08080 );
const gwColor gwWebSalmon       = _hex2bgra_( 0xFA8072 );
const gwColor gwWebIndianRed    = _hex2bgra_( 0xCD5C5C );
const gwColor gwWebRed          = _hex2bgra_( 0xFF0000 );
const gwColor gwWebCrimson      = _hex2bgra_( 0xDC143C );
const gwColor gwWebFireBrick    = _hex2bgra_( 0xB22222 );
const gwColor gwWebBrown        = _hex2bgra_( 0xA52A2A );
const gwColor gwWebDarkRed      = _hex2bgra_( 0x8B0000 );
const gwColor gwWebMaroon       = _hex2bgra_( 0x800000 );

/*Pink */
const gwColor gwWebMistyRose        = _hex2bgra_( 0xFFE4E1 );
const gwColor gwWebPink             = _hex2bgra_( 0xFFC0CB );
const gwColor gwWebLightPink        = _hex2bgra_( 0xFFB6C1 );
const gwColor gwWebHotPink          = _hex2bgra_( 0xFF69B4 );
const gwColor gwWebRosyBrown        = _hex2bgra_( 0xBC8F8F );
const gwColor gwWebPaleVioletRed    = _hex2bgra_( 0xDB7093 );
const gwColor gwWebDeepPink         = _hex2bgra_( 0xFF1493 );
const gwColor gwWebMediumVioletRed  = _hex2bgra_( 0xC71585 );
                                                           
/* Oranges */                                              
const gwColor gwWebPapayaWhip       = _hex2bgra_( 0xFFEFD5 );
const gwColor gwWebBlanchedAlmond   = _hex2bgra_( 0xFFEBCD );
const gwColor gwWebBisque           = _hex2bgra_( 0xFFE4C4 );
const gwColor gwWebMoccasin         = _hex2bgra_( 0xFFE4B5 );
const gwColor gwWebPeachPuff        = _hex2bgra_( 0xFFDAB9 );
const gwColor gwWebNavajoWhite      = _hex2bgra_( 0xFFDEAD );
const gwColor gwWebLightSalmon      = _hex2bgra_( 0xFFA07A );
const gwColor gwWebDarkSalmon       = _hex2bgra_( 0xE9967A );
const gwColor gwWebOrange           = _hex2bgra_( 0xFFA500 );
const gwColor gwWebDarkOrange       = _hex2bgra_( 0xFF8C00 );
const gwColor gwWebCoral            = _hex2bgra_( 0xFF7F50 );
const gwColor gwWebTomato           = _hex2bgra_( 0xFF6347 );
const gwColor gwWebOrangeRed        = _hex2bgra_( 0xFF4500 );
                                                           
/* Marrons */                                              
const gwColor gwWebWheat            = _hex2bgra_( 0xF5DEB3 );
const gwColor gwWebBurlyWood        = _hex2bgra_( 0xDEB887 );
const gwColor gwWebTan              = _hex2bgra_( 0xD2B48C );
const gwColor gwWebSandyBrown       = _hex2bgra_( 0xF4A460 );
const gwColor gwWebGoldenrod        = _hex2bgra_( 0xDAA520 );
const gwColor gwWebPeru             = _hex2bgra_( 0xCD853F );
const gwColor gwWebDarkGoldenrod    = _hex2bgra_( 0xB8860B );
const gwColor gwWebChocolate        = _hex2bgra_( 0xD2691E );
const gwColor gwWebSienna           = _hex2bgra_( 0xA0522D );
const gwColor gwWebSaddleBrown      = _hex2bgra_( 0x8B4513 );
                                                          
/* Yellows */                                                  
const gwColor gwWebLightYellow          = _hex2bgra_( 0xFFFFE0 );
const gwColor gwWebCornsilk             = _hex2bgra_( 0xFFF8DC );
const gwColor gwWebLemonChiffon         = _hex2bgra_( 0xFFFACD );
const gwColor gwWebLightGoldenrodYellow = _hex2bgra_( 0xFAFAD2 );
const gwColor gwWebPaleGoldenrod        = _hex2bgra_( 0xEEE8AA );
const gwColor gwWebKhaki                = _hex2bgra_( 0xF0E68C );
const gwColor gwWebYellow               = _hex2bgra_( 0xFFFF00 );
const gwColor gwWebGold                 = _hex2bgra_( 0xFFD700 );
const gwColor gwWebDarkKhaki            = _hex2bgra_( 0xBDB76B );
                                                               
/* Green Yellows */                                            
const gwColor gwWebGreenYellow          = _hex2bgra_( 0xADFF2F );
const gwColor gwWebChartreuse           = _hex2bgra_( 0x7FFF00 ); 
const gwColor gwWebLawnGreen            = _hex2bgra_( 0x7CFC00 );
const gwColor gwWebYellowGreen          = _hex2bgra_( 0x9ACD32 );
const gwColor gwWebOliveDrab            = _hex2bgra_( 0x6B8E23 );
const gwColor gwWebOlive                = _hex2bgra_( 0x808000 );
const gwColor gwWebDarkOliveGreen       = _hex2bgra_( 0x556B2F );

/* Greens */
const gwColor gwWebPaleGreen            = _hex2bgra_( 0x98FB98 );
const gwColor gwWebLightGreen           = _hex2bgra_( 0x90EE90 );
const gwColor gwWebMediumSpringGreen    = _hex2bgra_( 0x00FA9A );
const gwColor gwWebSpringGreen          = _hex2bgra_( 0x00FF7F );
const gwColor gwWebLime                 = _hex2bgra_( 0x00FF00 );
const gwColor gwWebDarkSeaGreen         = _hex2bgra_( 0x8FBC8F );
const gwColor gwWebLimeGreen            = _hex2bgra_( 0x32CD32 );
const gwColor gwWebMediumSeaGreen       = _hex2bgra_( 0x3CB371 );
const gwColor gwWebSeaGreen             = _hex2bgra_( 0x2E8B57 );
const gwColor gwWebForestGreen          = _hex2bgra_( 0x228B22 );
const gwColor gwWebGreen                = _hex2bgra_( 0x008000 );
const gwColor gwWebDarkGreen            = _hex2bgra_( 0x006400 );
                                                               
/* Blue Greens */                                              
const gwColor gwWebLightCyan            = _hex2bgra_( 0xE0FFFF );
const gwColor gwWebPaleTurquoise        = _hex2bgra_( 0xAFEEEE );
const gwColor gwWebAquamarine           = _hex2bgra_( 0x7FFFD4 );
const gwColor gwWebAqua                 = _hex2bgra_( 0x00FFFF );
const gwColor gwWebCyan                 = _hex2bgra_( 0x00FFFF );
const gwColor gwWebTurquoise            = _hex2bgra_( 0x40E0D0 );
const gwColor gwWebMediumTurquoise      = _hex2bgra_( 0x48D1CC );
const gwColor gwWebDarkTurquoise        = _hex2bgra_( 0x00CED1 );
const gwColor gwWebMediumAquamarine     = _hex2bgra_( 0x66CDAA );
const gwColor gwWebLightSeaGreen        = _hex2bgra_( 0x20B2AA );
const gwColor gwWebCadetBlue            = _hex2bgra_( 0x5F9EA0 );
const gwColor gwWebDarkCyan             = _hex2bgra_( 0x008B8B );
const gwColor gwWebTeal                 = _hex2bgra_( 0x008080 );
                                                               
/* Blues */
const gwColor gwWebLavender         = _hex2bgra_( 0xE6E6FA );
const gwColor gwWebBlueWeb          = _hex2bgra_( 0xCEE7FF );
const gwColor gwWebPowderBlue       = _hex2bgra_( 0xB0E0E6 );
const gwColor gwWebLightBlue        = _hex2bgra_( 0xADD8E6 );
const gwColor gwWebLightSkyBlue     = _hex2bgra_( 0x87CEFA );
const gwColor gwWebSkyBlue          = _hex2bgra_( 0x87CEEB );
const gwColor gwWebLightSteelBlue   = _hex2bgra_( 0xB0C4DE );
const gwColor gwWebDeepSkyBlue      = _hex2bgra_( 0x00BFFF );
const gwColor gwWebCornflowerBlue   = _hex2bgra_( 0x6495ED );
const gwColor gwWebDodgerBlue       = _hex2bgra_( 0x1E90FF );
const gwColor gwWebSteelBlue        = _hex2bgra_( 0x4682B4 );
const gwColor gwWebRoyalBlue        = _hex2bgra_( 0x4169E1 );
const gwColor gwWebBlue             = _hex2bgra_( 0x0000FF );
const gwColor gwWebMediumBlue       = _hex2bgra_( 0x0000CD );
const gwColor gwWebDarkBlue         = _hex2bgra_( 0x00008B );
const gwColor gwWebNavy             = _hex2bgra_( 0x000080 );
const gwColor gwWebMidnightBlue     = _hex2bgra_( 0x191970 );

/* Violets */
const gwColor gwWebThistle          = _hex2bgra_( 0xD8BFD8 );
const gwColor gwWebPlum             = _hex2bgra_( 0xDDA0DD );
const gwColor gwWebViolet           = _hex2bgra_( 0xEE82EE );
const gwColor gwWebOrchid           = _hex2bgra_( 0xDA70D6 );
const gwColor gwWebFuchsia          = _hex2bgra_( 0xFF00FF );
const gwColor gwWebMagenta          = _hex2bgra_( 0xFF00FF );
const gwColor gwWebMediumPurple     = _hex2bgra_( 0x9370DB );
const gwColor gwWebMediumOrchid     = _hex2bgra_( 0xBA55D3 );
const gwColor gwWebMediumSlateBlue  = _hex2bgra_( 0x7B68EE );
const gwColor gwWebSlateBlue        = _hex2bgra_( 0x6A5ACD );
const gwColor gwWebBlueViolet       = _hex2bgra_( 0x8A2BE2 );
const gwColor gwWebDarkViolet       = _hex2bgra_( 0x9400D3 );
const gwColor gwWebDarkOrchid       = _hex2bgra_( 0x9932CC );
const gwColor gwWebDarkMagenta      = _hex2bgra_( 0x8B008B );
const gwColor gwWebPurple           = _hex2bgra_( 0x800080 );
const gwColor gwWebDarkSlateBlue    = _hex2bgra_( 0x483D8B );
const gwColor gwWebIndigo           = _hex2bgra_( 0x4B0082 );

/* Whites */
const gwColor gwWebWhite            = _hex2bgra_( 0xFFFFFF );
const gwColor gwWebWhiteSmoke       = _hex2bgra_( 0xF5F5F5 );
const gwColor gwWebSnow             = _hex2bgra_( 0xFFFAFA );
const gwColor gwWebSeashell         = _hex2bgra_( 0xFFF5EE );
const gwColor gwWebLinen            = _hex2bgra_( 0xFAF0E6 );
const gwColor gwWebAntiqueWhite     = _hex2bgra_( 0xFAEBD7 );
const gwColor gwWebOldLace          = _hex2bgra_( 0xFDF5E6 );
const gwColor gwWebFloralWhite      = _hex2bgra_( 0xFFFAF0 );
const gwColor gwWebIvory            = _hex2bgra_( 0xFFFFF0 );
const gwColor gwWebBeige            = _hex2bgra_( 0xF5F5DC );
const gwColor gwWebHoneydew         = _hex2bgra_( 0xF0FFF0 );
const gwColor gwWebMintCream        = _hex2bgra_( 0xF5FFFA );
const gwColor gwWebAzure            = _hex2bgra_( 0xF0FFFF );
const gwColor gwWebAliceBlue        = _hex2bgra_( 0xF0F8FF );
const gwColor gwWebGhostWhite       = _hex2bgra_( 0xF8F8FF );
const gwColor gwWebLavenderBlush    = _hex2bgra_( 0xFFF0F5 );

/* Greys */
const gwColor gwWebGainsboro        = _hex2bgra_( 0xDCDCDC );
const gwColor gwWebLightGrey        = _hex2bgra_( 0xD3D3D3 );
const gwColor gwWebSilver           = _hex2bgra_( 0xC0C0C0 );
const gwColor gwWebDarkGray         = _hex2bgra_( 0xA9A9A9 );
const gwColor gwWebLightSlateGray   = _hex2bgra_( 0x778899 );
const gwColor gwWebSlateGray        = _hex2bgra_( 0x708090 );
const gwColor gwWebGray             = _hex2bgra_( 0x808080 );
const gwColor gwWebDimGray          = _hex2bgra_( 0x696969 );
const gwColor gwWebDarkSlateGray    = _hex2bgra_( 0x2F4F4F );
const gwColor gwWebBlack            = _hex2bgra_( 0x000000 );

#endif /* _HGW_COLORS_H */
