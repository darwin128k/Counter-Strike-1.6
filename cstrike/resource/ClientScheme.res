//
// TRACKER SCHEME RESOURCE FILE
//
// sections:
//		colors			- all the colors used by the scheme
//		basesettings	- contains settings for app to use to draw controls
//		fonts			- list of all the fonts used by app
//		borders			- description of all the borders
//
// notes:
// 		hit ctrl-alt-shift-R in the app to reload this file
//
Scheme
{
	//Name - currently overriden in code
	//{
	//	"Name"	"ClientScheme"
	//}

	//////////////////////// COLORS ///////////////////////////
	Colors
	{
		// In-game VGUI (MOTD, team/class, buy). GoldSrc client reads THIS
		// file, not TrackerScheme. Keep CS 1.6 gold-on-transparent so
		// labels stay visible on the white HTML briefing.
		"BaseText"			"255 176 0 255"
		"BrightBaseText"		"255 176 0 255"
		"SelectedText"		"255 176 0 255"
		"DimBaseText"		"255 176 0 255"
		"LabelDimText"		"255 176 0 164"
		"ControlText"		"255 176 0 255"
		"BrightControlText"	"255 176 0 255"
		"DisabledText1"		"80 48 0 255"
		"DisabledText2"		"0 0 0 0"
		"DimListText"		"188 112 0 255"

		"ButtonFocusBorder"	"64 48 0 255"

		"ControlBG"			"0 0 0 0"
		"ButtonBG"			"0 0 0 64"
		"ControlDarkBG"		"0 0 0 128"
		"WindowBG"			"0 0 0 200"
		"SelectionBG"		"255 176 0 100"
		"SelectionBG2"		"255 255 255 25"
		"ListBG"			"0 0 0 128"
		"ViewportBG"		"0 0 0 200"
		"CareerBoxBG"		"0 0 0 227"
		"Menu/BgColor"		"0 0 0 227"

		"CreditsNormalFG"	"255 255 255 255"
		"CreditsNormalBG"	"0 0 0 255"
		"CreditsTitleFG"	"255 174 0 255"
		"CreditsTitleBG"	"0 0 0 255"
		"CreditsStudioTitleFG"	"255 174 0 255"
		"CreditsStudioTitleBG"	"0 0 0 255"
		"CreditsIndividualTitleFG"	"255 174 0 255"
		"CreditsIndividualTitleBG"	"0 0 0 255"

		"TitleText"			"255 174 0 255"
		"TitleDimText"		"255 174 0 255"
		"TitleBG"			"255 255 0 0"
		"TitleDimBG"		"255 255 0 0"
		"TitleIcon"		"255 174 0 255"

		"SliderTickColor"		"127 140 127 255"
		"SliderTrackColor"	"31 31 31 255"

		"BorderBright"		"188 112 0 128"
		"BorderDark"		"188 112 0 128"
		"BorderSelection"		"188 112 0 0"
		"BorderBuyPreset"		"255 176 0 255"

		"team0"			"204 204 204 255"
		"team1"			"255 64 64 255"
		"team2"			"153 204 255 255"

		"StatisticsText"		"255 255 255 255"
		"SelectedStatisticsText"	"128 128 128 255"
		"BuyPresetBG"			"0 0 0 64"
		"BuyPresetAltBG"		"0 0 0 128"
		"MapDescriptionText"	"255 176 0 255"
	}

	///////////////////// BASE SETTINGS ////////////////////////
	// default settings for all panels
	// controls use these to determine their settings
	BaseSettings
	{
		"FgColor"			"ControlText"
		"BgColor"			"ControlBG"
		"ButtonBgColor"			"ButtonBG"
		"LabelBgColor"		"ControlBG"
		"SubPanelBgColor"	"ControlBG"

		"DisabledFgColor1"		"DisabledText1" 
		"DisabledFgColor2"		"DisabledText2"			// set this to the BgColor if you don't want it to draw

		"TitleBarFgColor"			"TitleText"
		"TitleBarDisabledFgColor"	"TitleDimText"
		"TitleBarBgColor"			"TitleBG"
		"TitleBarDisabledBgColor"	"TitleDimBG"

		"TitleBarIcon"				"resource/icon_steam"
		"TitleBarDisabledIcon"		"resource/icon_steam_disabled"

		"TitleButtonFgColor"			"BorderBright"
		"TitleButtonBgColor"			"ControlBG"
		"TitleButtonDisabledFgColor"	"TitleDimText"
		"TitleButtonDisabledBgColor"	"TitleDimBG"

		"TextCursorColor"			"BaseText"			// color of the blinking text cursor in text entries
		"URLTextColor"				"BrightBaseText"	// color that URL's show up in chat window

		Menu
		{
			"FgColor"			"DimBaseText"
			"BgColor"			"ControlBG"
			"ArmedFgColor"		"BrightBaseText"
			"ArmedBgColor"		"SelectionBG"
			"DividerColor"		"BorderDark"
			"TextInset"			"6"
		}

		MenuButton	  // the little arrow on the side of boxes that triggers drop down menus
		{
			"ButtonArrowColor"	"DimBaseText"		// color of arrows
		   	"ButtonBgColor"		"WindowBG"			// bg color of button. same as background color of text edit panes 
			"ArmedArrowColor"	"BrightBaseText"	// color of arrow when mouse is over button
			"ArmedBgColor"		"DimBaseText"		// bg color of button when mouse is over button
		}

		Slider
		{
			"SliderFgColor"		"ControlBG"			// handle with which the slider is grabbed
			"SliderBgColor"		"ControlDarkBG"		// area behind handle
		}

		ScrollBarSlider
		{
			"BgColor"					"ControlBG"		// this isn't really used
			"ScrollBarSliderFgColor"	"ControlBG"		// handle with which the slider is grabbed
			"ScrollBarSliderBgColor"	"ControlDarkBG"	// area behind handle
			"ButtonFgColor"				"DimBaseText"	// color of arrows
		}


		// text edit windows
		"WindowFgColor"				"BaseText"		// off-white
		"WindowBgColor"				"WindowBG"		// redundant. can we get rid of WindowBgColor and just use WindowBG?
		"WindowDisabledFgColor"		"DimBaseText"
		"WindowDisabledBgColor"		"ListBG"		// background of chat conversation
		"SelectionFgColor"			"SelectedText"	// fg color of selected text
		"SelectionBgColor"			"SelectionBG"
		"ListSelectionFgColor"		"SelectedText"
		"ListBgColor"				"ListBG"		// background of server browser control, etc
		"BuddyListBgColor"			"ListBG"		// background of buddy list pane
		
		// App-specific stuff
		"ChatBgColor"				"WindowBG"

		// status selection
		"StatusSelectFgColor"		"BrightBaseText"
		"StatusSelectFgColor2"		"BrightControlText"	// this is the color of the friends status

		// checkboxes
		"CheckButtonBorder1"   		"BorderDark"		// the left checkbutton border
		"CheckButtonBorder2"   		"BorderBright"		// the right checkbutton border
		"CheckButtonCheck"			"BrightControlText"	// color of the check itself
		"CheckBgColor"				"ListBG"

		// buttons (default fg/bg colors are used if these are not set)
//		"ButtonArmedFgColor"
		"ButtonArmedBgColor"		"SelectionBG"
//		"ButtonDepressedFgColor"	"BrightControlText"
//		"ButtonDepressedBgColor"

		// buddy buttons
		BuddyButton
		{
			"FgColor1"				"ControlText"
			"FgColor2"				"DimListText"
			"ArmedFgColor1"			"BrightBaseText"
			"ArmedFgColor2"			"BrightBaseText"
			"ArmedBgColor"			"SelectionBG"
		}

		Chat
		{
			"TextColor"				"BrightControlText"
			"SelfTextColor"			"BaseText"
			"SeperatorTextColor"	"DimBaseText"
		}

		Statistics
		{
			"Font"			"DefaultSmall"
			"TextColor"		"StatisticsText"
			"SelectedTextColor"	"SelectedStatisticsText"
		}

		"SectionTextColor"		"BrightControlText"	// text color for IN-GAME, ONLINE, OFFLINE sections of buddy list
		"SectionDividerColor"	"BorderDark"		// color of line that runs under section name in buddy list
	}

	//
	//////////////////////// FONTS /////////////////////////////
	//
	// describes all the fonts
	CustomFontFiles
	{
		"1"		"resource/Inter-Regular.ttf"
		"2"		"resource/Inter-Bold.ttf"
		"3"		"resource/Inter-SemiBold.ttf"
	}

	Fonts
	{
		// fonts are used in order that they are listed
		// fonts listed later in the order will only be used if they fulfill a range not already filled
		// if a font fails to load then the subsequent fonts will replace
		"Default"
		{
			"1"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"12"
				"weight"	"600"
				"yres"	"480 599"
				"antialias"	"1"
			}
			"2"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"13"
				"weight"	"600"
				"yres"	"600 767"
				"antialias"	"1"
			}
			"3"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"14"
				"weight"	"600"
				"yres"	"768 1023"
				"antialias"	"1"
			}
			"4"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"20"
				"weight"	"600"
				"yres"	"1024 1199"
				"antialias"	"1"
			}
			"5"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"24"
				"weight"	"600"
				"yres"	"1200 6000"
				"antialias"	"1"
			}
		}
		"DefaultUnderline"
		{
			"1"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"12"
				"weight"	"500"
				"underline" "1"
				"antialias"	"1"
			}
		}
		"DefaultSmall"
		{
			"1"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"12"
				"weight"	"0"
				"yres"	"480 599"
				"antialias"	"1"
			}
			"2"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"13"
				"weight"	"0"
				"yres"	"600 767"
				"antialias"	"1"
			}
			"3"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"14"
				"weight"	"0"
				"yres"	"768 1023"
				"antialias"	"1"
			}
			"4"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"20"
				"weight"	"0"
				"yres"	"1024 1199"
				"antialias"	"1"
			}
			"5"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"24"
				"weight"	"0"
				"yres"	"1200 6000"
				"antialias"	"1"
			}
		}
		"DefaultVerySmall"
		{
			"1"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"12"
				"weight"	"600"
				"yres"	"480 599"
				"antialias"	"1"
			}
			"2"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"13"
				"weight"	"600"
				"yres"	"600 767"
				"antialias"	"1"
			}
			"3"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"14"
				"weight"	"600"
				"yres"	"768 1023"
				"antialias"	"1"
			}
			"4"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"20"
				"weight"	"600"
				"yres"	"1024 1199"
				"antialias"	"1"
			}
			"5"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"24"
				"weight"	"600"
				"yres"	"1200 6000"
				"antialias"	"1"
			}
		}
		// Used by scoreboard and spectator UI for names which don't map in the normal fashion
		"DefaultVerySmallFallBack"
		{
			"1"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"10"
				"weight"	"0"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"		"480 599"
				"antialias"	"1"
			}
			"2"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"12"
				"weight"	"0"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"		"600 1199"
				"antialias"	"1"
			}
			"3"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"15"
				"weight"	"0"
				"range"		"0x0000 0x017F" //	Basic Latin, Latin-1 Supplement, Latin Extended-A
				"yres"		"1200 6000"
				"antialias"	"1"
			}
		}
		"Title"
		{
			"1"
			{
				"name"		"Inter Bold"
				"custom"	"1"
				"tall"		"18"
				"weight"	"500"
				"antialias"	"1"
			}
			"2"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"16"
				"weight"	"600"
				"antialias"	"1"
			}
		}
		// Credits ------------------------------------------------
		"CreditsTitle"
		{
			"1"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"26"
				"weight"	"800"
				"antialias"	"1"
			}
			"2"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"26"
				"weight"	"700"
				"antialias"	"1"
			}
		}
		"CreditsStudioTitle"
		{
			"1"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"24"
				"weight"	"800"
				"antialias"	"1"
			}
			"2"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"24"
				"weight"	"700"
				"antialias"	"1"
			}
		}
		"CreditsIndividualTitle"
		{
			"1"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"22"
				"weight"	"800"
				"antialias"	"1"
			}
			"2"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"22"
				"weight"	"700"
				"antialias"	"1"
			}
		}
		"CreditsNormal"
		{
			"1"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"20"
				"weight"	"800"
				"antialias"	"1"
			}
			"2"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"20"
				"weight"	"700"
				"antialias"	"1"
			}
		}
		// --------------------------------------------------------
		"DialogTitle"
		{
			"1"
			{
				"name"		"Inter Bold"
				"custom"	"1"
				"tall"		"14"
				"weight"	"500"
				"antialias"	"1"
			}
			"2"
			{
				"name"		"Inter"
				"custom"	"1"
				"tall"		"13"
				"weight"	"600"
				"antialias"	"1"
			}
		}

		// this is the symbol font
		"Marlett"
		{
			"1"
			{
				"name"		"Marlett"
				"tall"		"11"
				"weight"	"0"
				"symbol"	"1"
			}
		}
	}

	//
	//////////////////// BORDERS //////////////////////////////
	//
	// describes all the border types
	Borders
	{
		BaseBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "BorderDark"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "BorderBright"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "BorderDark"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "BorderBright"
					"offset" "0 0"
				}
			}
		}
		
		TitleButtonBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "BorderBright"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "BorderDark"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "BorderBright"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "BorderDark"
					"offset" "0 0"
				}
			}
		}

		TitleButtonDisabledBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "BgColor"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "BgColor"
					"offset" "1 0"
				}
			}
			Top
			{
				"1"
				{
					"color" "BgColor"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "BgColor"
					"offset" "0 0"
				}
			}
		}

		TitleButtonDepressedBorder
		{
			"inset" "1 1 1 1"
			Left
			{
				"1"
				{
					"color" "BorderDark"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "BorderBright"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "BorderDark"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "BorderBright"
					"offset" "0 0"
				}
			}
		}

		ScrollBarButtonBorder
		{
			"inset" "1 0 0 0"
			Left
			{
				"1"
				{
					"color" "BorderBright"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "BorderDark"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "BorderBright"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "BorderDark"
					"offset" "0 0"
				}
			}
		}

		ScrollBarButtonDepressedBorder
		{
			"inset" "2 2 0 0"
			Left
			{
				"1"
				{
					"color" "BorderDark"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "BorderBright"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "BorderDark"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "BorderBright"
					"offset" "0 0"
				}
			}
		}
		
		ButtonBorder
		{
		}

		BuyPresetBorder
		{
			"inset" "0 0 0 0"
			Left
			{
				"1"
				{
					"color" "BorderBuyPreset"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "BorderBuyPreset"
					"offset" "0 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "BorderBuyPreset"
					"offset" "1 1"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "BorderBuyPreset"
					"offset" "0 0"
				}
			}
		}

		FrameBorder
		{
		}

		TabBorder
		{
		}

		TabActiveBorder
		{
		}


		ToolTipBorder
		{
			"inset" "0 0 1 0"
			Left
			{
				"1"
				{
					"color" "BorderDark"
					"offset" "0 0"
				}
			}

			Right
			{
				"1"
				{
					"color" "BorderDark"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "BorderDark"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "BorderDark"
					"offset" "0 0"
				}
			}
		}

		// this is the border used for default buttons (the button that gets pressed when you hit enter)
		ButtonKeyFocusBorder
		{
		}

		ButtonDepressedBorder
		{
		}

		ComboBoxBorder
		{
			"inset" "0 0 1 1"
			Left
			{
				"1"
				{
					"color" "BorderDark"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "BorderBright"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "BorderDark"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "BorderBright"
					"offset" "0 0"
				}
			}
		}

		MenuBorder
		{
			"inset" "1 1 1 1"
			Left
			{
				"1"
				{
					"color" "BorderBright"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "BorderDark"
					"offset" "1 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "BorderBright"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "BorderDark"
					"offset" "0 0"
				}
			}
		}
		BrowserBorder
		{
			"inset" "0 0 0 1"
			Left
			{
				"1"
				{
					"color" "BorderDark"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "BorderBright"
					"offset" "0 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "BorderDark"
					"offset" "0 0"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "BorderBright"
					"offset" "0 0"
				}
			}
		}

		BuyPresetButtonBorder
		{
			"inset" "0 0 0 0"
			Left
			{
				"1"
				{
					"color" "ControlBG"
					"offset" "0 1"
				}
			}

			Right
			{
				"1"
				{
					"color" "BorderDark"
					"offset" "0 0"
				}
			}

			Top
			{
				"1"
				{
					"color" "BorderBright"
					"offset" "1 1"
				}
			}

			Bottom
			{
				"1"
				{
					"color" "ControlBG"
					"offset" "0 0"
				}
			}
		}
	}
}
