// Framework includes
#include "BsApplication.h"
#include "Resources/BsResources.h"
#include "Resources/BsBuiltinResources.h"
#include "Material/BsMaterial.h"
#include "Components/BsCCamera.h"
#include "GUI/BsCGUIWidget.h"
#include "GUI/BsGUIPanel.h"
#include "GUI/BsGUILayoutX.h"
#include "GUI/BsGUILayoutY.h"
#include "GUI/BsGUILabel.h"
#include "GUI/BsGUIButton.h"
#include "GUI/BsGUIInputBox.h"
#include "GUI/BsGUIListBox.h"
#include "GUI/BsGUIToggle.h"
#include "GUI/BsGUIScrollArea.h"
#include "GUI/BsGUISpace.h"
#include "RenderAPI/BsRenderAPI.h"
#include "RenderAPI/BsRenderWindow.h"
#include "Scene/BsSceneObject.h"
#include "BsExampleFramework.h"
#include "Image/BsSpriteTexture.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This example demonstrates how to set up a graphical user interface. It demoes a variety of common GUI elements, as well
// as demonstrating the capability of layouts. It also shows how to customize the look of GUI elements. 
//
// The example starts off by setting up a camera, which is required for any kind of rendering, including GUI. It then
// proceeds to demonstrate a set of basic controls, while using manual positioning. It then shows how to create a custom
// style and apply it to a GUI element. It follows to demonstrate the concept of layouts that automatically position
// and size elements, as well as scroll areas. Finally, it demonstrates a more complex example of creating a custom style,
// by creating a button with custom textures and font.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace bs
{
	u32 windowResWidth = 1280;
	u32 windowResHeight = 720;

	/** Set up the GUI elements and the camera. */
	void setUpGUI()
	{
		/************************************************************************/
		/* 									CAMERA	                     		*/
		/************************************************************************/

		// In order something to render on screen we need at least one camera.

		// Like before, we create a new scene object at (0, 0, 0).
		HSceneObject sceneCameraSO = SceneObject::Create("SceneCamera");

		// Get the primary render window we need for creating the camera. 
		SPtr<RenderWindow> window = gApplication().GetPrimaryWindow();

		// Add a Camera component that will output whatever it sees into that window 
		// (You could also use a render texture or another window you created).
		HCamera sceneCamera = sceneCameraSO->AddComponent<CCamera>();
		sceneCamera->GetViewport()->SetTarget(window);

		// Pick a prettier background color
		Color gray = Color(51/255.0f, 51/255.0f, 51/255.0f);
		sceneCamera->GetViewport()->SetClearColorValue(gray);

		// Let the camera know it will be used for overlay rendering only. This stops the renderer from running potentially
		// expensive effects that ultimately don't effect anything. It also allows us to use a linear-space color for the
		// camera background (normal rendering expects colors in gamma space, which is unintuitive for aspects such as 
		// GUI).
		const SPtr<RenderSettings>& renderSettings = sceneCamera->GetRenderSettings();
		renderSettings->OverlayOnly = true;
		sceneCamera->SetRenderSettings(renderSettings);

		/************************************************************************/
		/* 									GUI		                     		*/
		/************************************************************************/
		// Add a GUIWidget component we will use for rendering the GUI
		HSceneObject guiSO = SceneObject::Create("GUI");
		HGUIWidget gui = guiSO->AddComponent<CGUIWidget>(sceneCamera);

		// Retrieve the primary panel onto which to attach GUI elements to. Panels allow free placement of elements in
		// them (unlike layouts), and can also have depth, meaning you can overlay multiple panels over one another.
		GUIPanel* mainPanel = gui->GetPanel();

		////////////////////// Add a variety of GUI controls /////////////////////
		// Clickable button with a textual label
		GUIButton* button = mainPanel->AddNewElement<GUIButton>(HString("Click me!"));
		button->OnClick.Connect([]()
		{
			// Log a message when the user clicks the button
			BS_LOG(Info, Uncategorized, "Button clicked!");
		});

		button->SetPosition(10, 50);
		button->SetSize(100, 30);

		// Toggleable button
		GUIToggle* toggle = mainPanel->AddNewElement<GUIToggle>(HString(""));
		toggle->OnToggled.Connect([](bool enabled)
		{
			// Log a message when the user toggles the button
			if(enabled)
			{
				BS_LOG(Info, Uncategorized, "Toggle turned on");
			}
			else
			{
				BS_LOG(Info, Uncategorized, "Toggle turned off");
			}
		});

		toggle->SetPosition(10, 90);

		// Add non-interactable label next to the toggle
		GUILabel* toggleLabel = mainPanel->AddNewElement<GUILabel>(HString("Toggle me!"));
		toggleLabel->SetPosition(30, 92);

		// Single-line box in which user can input text into
		GUIInputBox* inputBox = mainPanel->AddNewElement<GUIInputBox>();
		inputBox->OnValueChanged.Connect([](const String& value)
		{
			// Log a message when the user enters new text in the input box
			BS_LOG(Info, Uncategorized, "User entered: \"" + value + "\"");
		});

		inputBox->SetText("Type in me...");
		inputBox->SetPosition(10, 115);
		inputBox->SetWidth(100);

		// List box allowing you to select one of the specified elements
		Vector<HString> listBoxElements =
		{
			HString("Blue"),
			HString("Black"),
			HString("Green"),
			HString("Orange")
		};

		GUIListBox* listBox = mainPanel->AddNewElement<GUIListBox>(listBoxElements);
		listBox->OnSelectionToggled.Connect([listBoxElements](u32 idx, bool enabled)
		{
			// Log a message when the user selects a new element
			BS_LOG(Info, Uncategorized, "User selected element: \"" + listBoxElements[idx].GetValue() + "\"");
			
		});

		listBox->SetPosition(10, 140);
		listBox->SetWidth(100);

		// Add a button with an image
		HTexture icon = ExampleFramework::LoadTexture(ExampleTexture::GUIBansheeIcon, false, false, false, false);
		HSpriteTexture iconSprite = SpriteTexture::Create(icon);

		// Create a GUI content object that contains an icon to display on the button. Also an optional text and tooltip.
		GUIContent buttonContent(iconSprite);
		GUIButton* iconButton = mainPanel->AddNewElement<GUIButton>(buttonContent);

		iconButton->SetPosition(10, 170);
		iconButton->SetSize(70, 70);

		/////////////////////////// Header label /////////////////////////////////
		// Create a custom style for a label we'll used for headers. Then add a header
		// for the controls we added in the previous section.

		// Create a new style
		GUIElementStyle headerLabelStyle;

		// Make it use a custom font with size 30
		headerLabelStyle.Font = ExampleFramework::LoadFont(ExampleFont::SegoeUISemiBold, { 24 });
		headerLabelStyle.FontSize = 24;

		// Set the default text color
		headerLabelStyle.Normal.TextColor = Color::White;

		// Grab the default GUI skin to which we'll append the new style to. You could also create a new GUI skin and
		// add the style to it, but that would also require adding default styles for all the GUI element types.
		HGUISkin skin = gBuiltinResources().GetGuiSkin();
		skin->SetStyle("HeaderLabelStyle", headerLabelStyle);

		// Create and position the label
		GUILabel* basicControlsLbl = mainPanel->AddNewElement<GUILabel>(HString("Basic controls"), "HeaderLabelStyle");
		basicControlsLbl->SetPosition(10, 10);

		///////////////////////////  vertical layout /////////////////////////
		// Use a vertical layout to automatically position GUI elements. This is unlike above where we position and
		// sized all elements manually.
		GUILayoutY* vertLayout = mainPanel->AddNewElement<GUILayoutY>();

		// Add five buttons to the layout
		for(u32 i = 0; i < 5; i++)
		{
			vertLayout->AddNewElement<GUIButton>(HString("Click me!"));

			// Add a 10 pixel spacing between each button
			vertLayout->AddNewElement<GUIFixedSpace>(10);
		}

		// Add a flexible space ensuring all the elements get pushed to the top of the layout
		vertLayout->AddNewElement<GUIFlexibleSpace>();

		// Position the layout relative to the main panel, and limit width to 100 pixels
		vertLayout->SetPosition(350, 50);
		vertLayout->SetWidth(100);

		// Add a header
		GUILabel* vertLayoutLbl = mainPanel->AddNewElement<GUILabel>(HString("Vertical layout"), "HeaderLabelStyle");
		vertLayoutLbl->SetPosition(300, 10);

		////////////////////////// Horizontal layout ///////////////////////
		// Use a horizontal layout to automatically position GUI elements
		GUILayoutX* horzLayout = mainPanel->AddNewElement<GUILayoutX>();
		horzLayout->AddNewElement<GUIFlexibleSpace>();

		// Add vive buttons to the layout
		for(u32 i = 0; i < 5; i++)
		{
			horzLayout->AddNewElement<GUIButton>(HString("Click me!"));
			horzLayout->AddNewElement<GUIFlexibleSpace>();
		}

		// Position the layout relative to the main panel, and limit the height to 30 pixels
		horzLayout->SetPosition(0, 340);
		horzLayout->SetHeight(30);

		// Add a header
		GUILabel* horzLayoutLbl = mainPanel->AddNewElement<GUILabel>(HString("Horizontal layout"), "HeaderLabelStyle");
		horzLayoutLbl->SetPosition(10, 300);

		//////////////////////////// Scroll area ///////////////////////
		// Container GUI element that allows scrolling if the number of elements inside the area are larger than the visible
		// area
		GUIScrollArea* scrollArea = mainPanel->AddNewElement<GUIScrollArea>();

		// Scroll areas have a vertical layout we can append elements to, same as with a normal layout
		GUILayout& scrollLayout = scrollArea->GetLayout();

		for(u32 i = 0; i < 15; i++)
			scrollLayout.AddNewElement<GUIButton>(HString("Click me!"));

		scrollArea->SetPosition(565, 50);
		scrollArea->SetSize(130, 200);

		// Add a header
		GUILabel* scrollAreaLbl = mainPanel->AddNewElement<GUILabel>(HString("Scroll area"), "HeaderLabelStyle");
		scrollAreaLbl->SetPosition(550, 10);

		///////////////////////////// Button using a custom style ///////////////////
		HTexture buttonNormalTex = ExampleFramework::LoadTexture(ExampleTexture::GUIExampleButtonNormal, false, false, 
			false, false);
		HTexture buttonHoverTex = ExampleFramework::LoadTexture(ExampleTexture::GUIExampleButtonHover, false, false,  
			false, false);
		HTexture buttonActiveTex = ExampleFramework::LoadTexture(ExampleTexture::GUIExampleButtonActive, false, false, 
			false, false);

		// Create a new style
		GUIElementStyle customBtnStyle;

		// Set custom textures for 'normal', 'hover' and 'active' states of the button
		customBtnStyle.Normal.Texture = SpriteTexture::Create(buttonNormalTex);
		customBtnStyle.Hover.Texture = SpriteTexture::Create(buttonHoverTex);
		customBtnStyle.Active.Texture = SpriteTexture::Create(buttonActiveTex);

		// Button size is fixed, and should match the size of the texture's we're using
		customBtnStyle.FixedHeight = true;
		customBtnStyle.FixedWidth = true;
		customBtnStyle.Width = buttonNormalTex->GetProperties().GetWidth();
		customBtnStyle.Height = buttonNormalTex->GetProperties().GetHeight();

		// Make the button use a custom font for text
		customBtnStyle.Font = ExampleFramework::LoadFont(ExampleFont::SegoeUILight, { 24 });
		customBtnStyle.FontSize = 24;

		// Offset the position of the text within the button, to match the texture
		customBtnStyle.ContentOffset.Top = 20;
		customBtnStyle.ContentOffset.Left = 15;
		customBtnStyle.ContentOffset.Right = 65;

		skin->SetStyle("CustomButtonStyle", customBtnStyle);

		// Create the button that uses the custom style
		GUIButton* customButton = mainPanel->AddNewElement<GUIButton>(HString("Click me!"), "CustomButtonStyle");
		customButton->SetPosition(800, 50);

		// Add a header
		GUILabel* customButtonLbl = mainPanel->AddNewElement<GUILabel>(HString("Custom button"), "HeaderLabelStyle");
		customButtonLbl->SetPosition(800, 10);
	}
}

/** Main entry point into the application. */
#if BS_PLATFORM == BS_PLATFORM_WIN32
#include <windows.h>

int CALLBACK WinMain(
	_In_  HINSTANCE hInstance,
	_In_  HINSTANCE hPrevInstance,
	_In_  LPSTR lpCmdLine,
	_In_  int nCmdShow
	)
#else
int main()
#endif
{
	using namespace bs;

	// Initializes the application and creates a window with the specified properties
	VideoMode videoMode(windowResWidth, windowResHeight);
	Application::StartUp(videoMode, "Example", false);

	// Load a resource manifest so previously saved Fonts can find their child Texture resources
	ExampleFramework::LoadResourceManifest();

	// Set up the GUI elements
	setUpGUI();

	// Save the manifest, in case we did any asset importing during the setup stage
	ExampleFramework::SaveResourceManifest();

	// Runs the main loop that does most of the work. This method will exit when user closes the main
	// window or exits in some other way.
	Application::Instance().RunMainLoop();

	// When done, clean up
	Application::ShutDown();

	return 0;
}
