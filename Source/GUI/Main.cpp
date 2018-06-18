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
	UINT32 windowResWidth = 1280;
	UINT32 windowResHeight = 720;

	/** Set up the GUI elements and the camera. */
	void setUpGUI()
	{
		/************************************************************************/
		/* 									CAMERA	                     		*/
		/************************************************************************/

		// In order something to render on screen we need at least one camera.

		// Like before, we create a new scene object at (0, 0, 0).
		HSceneObject sceneCameraSO = SceneObject::create("SceneCamera");

		// Get the primary render window we need for creating the camera. 
		SPtr<RenderWindow> window = gApplication().getPrimaryWindow();

		// Add a Camera component that will output whatever it sees into that window 
		// (You could also use a render texture or another window you created).
		HCamera sceneCamera = sceneCameraSO->addComponent<CCamera>();
		sceneCamera->getViewport()->setTarget(window);

		// Pick a prettier background color
		Color gray = Color(51/255.0f, 51/255.0f, 51/255.0f);
		sceneCamera->getViewport()->setClearColorValue(gray);

		// Let the camera know it will be used for overlay rendering only. This stops the renderer from running potentially
		// expensive effects that ultimately don't effect anything. It also allows us to use a linear-space color for the
		// camera background (normal rendering expects colors in gamma space, which is unintuitive for aspects such as 
		// GUI).
		const SPtr<RenderSettings>& renderSettings = sceneCamera->getRenderSettings();
		renderSettings->overlayOnly = true;
		sceneCamera->setRenderSettings(renderSettings);

		/************************************************************************/
		/* 									GUI		                     		*/
		/************************************************************************/
		// Add a GUIWidget component we will use for rendering the GUI
		HSceneObject guiSO = SceneObject::create("GUI");
		HGUIWidget gui = guiSO->addComponent<CGUIWidget>(sceneCamera);

		// Retrieve the primary panel onto which to attach GUI elements to. Panels allow free placement of elements in
		// them (unlike layouts), and can also have depth, meaning you can overlay multiple panels over one another.
		GUIPanel* mainPanel = gui->getPanel();

		////////////////////// Add a variety of GUI controls /////////////////////
		// Clickable button with a textual label
		GUIButton* button = mainPanel->addNewElement<GUIButton>(HString("Click me!"));
		button->onClick.connect([]()
		{
			// Log a message when the user clicks the button
			LOGDBG("Button clicked!");
		});

		button->setPosition(10, 50);
		button->setSize(100, 30);

		// Toggleable button
		GUIToggle* toggle = mainPanel->addNewElement<GUIToggle>(HString(""));
		toggle->onToggled.connect([](bool enabled)
		{
			// Log a message when the user toggles the button
			if(enabled)
			{
				LOGDBG("Toggle turned on");
			}
			else
			{
				LOGDBG("Toggle turned off");
			}
		});

		toggle->setPosition(10, 90);

		// Add non-interactable label next to the toggle
		GUILabel* toggleLabel = mainPanel->addNewElement<GUILabel>(HString("Toggle me!"));
		toggleLabel->setPosition(30, 92);

		// Single-line box in which user can input text into
		GUIInputBox* inputBox = mainPanel->addNewElement<GUIInputBox>();
		inputBox->onValueChanged.connect([](const String& value)
		{
			// Log a message when the user enters new text in the input box
			LOGDBG("User entered: \"" + value + "\"");
		});

		inputBox->setText("Type in me...");
		inputBox->setPosition(10, 115);
		inputBox->setWidth(100);

		// List box allowing you to select one of the specified elements
		Vector<HString> listBoxElements =
		{
			HString("Blue"),
			HString("Black"),
			HString("Green"),
			HString("Orange")
		};

		GUIListBox* listBox = mainPanel->addNewElement<GUIListBox>(listBoxElements);
		listBox->onSelectionToggled.connect([listBoxElements](UINT32 idx, bool enabled)
		{
			// Log a message when the user selects a new element
			LOGDBG("User selected element: \"" + listBoxElements[idx].getValue() + "\"");
			
		});

		listBox->setPosition(10, 140);
		listBox->setWidth(100);

		// Add a button with an image
		HTexture icon = ExampleFramework::loadTexture(ExampleTexture::GUIBansheeIcon, false, false, false, false);
		HSpriteTexture iconSprite = SpriteTexture::create(icon);

		// Create a GUI content object that contains an icon to display on the button. Also an optional text and tooltip.
		GUIContent buttonContent(iconSprite);
		GUIButton* iconButton = mainPanel->addNewElement<GUIButton>(buttonContent);

		iconButton->setPosition(10, 170);
		iconButton->setSize(70, 70);

		/////////////////////////// Header label /////////////////////////////////
		// Create a custom style for a label we'll used for headers. Then add a header
		// for the controls we added in the previous section.

		// Create a new style
		GUIElementStyle headerLabelStyle;

		// Make it use a custom font with size 30
		headerLabelStyle.font = ExampleFramework::loadFont(ExampleFont::SegoeUISemiBold, { 24 });
		headerLabelStyle.fontSize = 24;

		// Set the default text color
		headerLabelStyle.normal.textColor = Color::White;

		// Grab the default GUI skin to which we'll append the new style to. You could also create a new GUI skin and
		// add the style to it, but that would also require adding default styles for all the GUI element types.
		HGUISkin skin = gBuiltinResources().getGUISkin();
		skin->setStyle("HeaderLabelStyle", headerLabelStyle);

		// Create and position the label
		GUILabel* basicControlsLbl = mainPanel->addNewElement<GUILabel>(HString("Basic controls"), "HeaderLabelStyle");
		basicControlsLbl->setPosition(10, 10);

		///////////////////////////  vertical layout /////////////////////////
		// Use a vertical layout to automatically position GUI elements. This is unlike above where we position and
		// sized all elements manually.
		GUILayoutY* vertLayout = mainPanel->addNewElement<GUILayoutY>();

		// Add five buttons to the layout
		for(UINT32 i = 0; i < 5; i++)
		{
			vertLayout->addNewElement<GUIButton>(HString("Click me!"));

			// Add a 10 pixel spacing between each button
			vertLayout->addNewElement<GUIFixedSpace>(10);
		}

		// Add a flexible space ensuring all the elements get pushed to the top of the layout
		vertLayout->addNewElement<GUIFlexibleSpace>();

		// Position the layout relative to the main panel, and limit width to 100 pixels
		vertLayout->setPosition(350, 50);
		vertLayout->setWidth(100);

		// Add a header
		GUILabel* vertLayoutLbl = mainPanel->addNewElement<GUILabel>(HString("Vertical layout"), "HeaderLabelStyle");
		vertLayoutLbl->setPosition(300, 10);

		////////////////////////// Horizontal layout ///////////////////////
		// Use a horizontal layout to automatically position GUI elements
		GUILayoutX* horzLayout = mainPanel->addNewElement<GUILayoutX>();
		horzLayout->addNewElement<GUIFlexibleSpace>();

		// Add vive buttons to the layout
		for(UINT32 i = 0; i < 5; i++)
		{
			horzLayout->addNewElement<GUIButton>(HString("Click me!"));
			horzLayout->addNewElement<GUIFlexibleSpace>();
		}

		// Position the layout relative to the main panel, and limit the height to 30 pixels
		horzLayout->setPosition(0, 340);
		horzLayout->setHeight(30);

		// Add a header
		GUILabel* horzLayoutLbl = mainPanel->addNewElement<GUILabel>(HString("Horizontal layout"), "HeaderLabelStyle");
		horzLayoutLbl->setPosition(10, 300);

		//////////////////////////// Scroll area ///////////////////////
		// Container GUI element that allows scrolling if the number of elements inside the area are larger than the visible
		// area
		GUIScrollArea* scrollArea = mainPanel->addNewElement<GUIScrollArea>();

		// Scroll areas have a vertical layout we can append elements to, same as with a normal layout
		GUILayout& scrollLayout = scrollArea->getLayout();

		for(UINT32 i = 0; i < 15; i++)
			scrollLayout.addNewElement<GUIButton>(HString("Click me!"));

		scrollArea->setPosition(565, 50);
		scrollArea->setSize(130, 200);

		// Add a header
		GUILabel* scrollAreaLbl = mainPanel->addNewElement<GUILabel>(HString("Scroll area"), "HeaderLabelStyle");
		scrollAreaLbl->setPosition(550, 10);

		///////////////////////////// Button using a custom style ///////////////////
		HTexture buttonNormalTex = ExampleFramework::loadTexture(ExampleTexture::GUIExampleButtonNormal, false, false, 
			false, false);
		HTexture buttonHoverTex = ExampleFramework::loadTexture(ExampleTexture::GUIExampleButtonHover, false, false,  
			false, false);
		HTexture buttonActiveTex = ExampleFramework::loadTexture(ExampleTexture::GUIExampleButtonActive, false, false, 
			false, false);

		// Create a new style
		GUIElementStyle customBtnStyle;

		// Set custom textures for 'normal', 'hover' and 'active' states of the button
		customBtnStyle.normal.texture = SpriteTexture::create(buttonNormalTex);
		customBtnStyle.hover.texture = SpriteTexture::create(buttonHoverTex);
		customBtnStyle.active.texture = SpriteTexture::create(buttonActiveTex);

		// Button size is fixed, and should match the size of the texture's we're using
		customBtnStyle.fixedHeight = true;
		customBtnStyle.fixedWidth = true;
		customBtnStyle.width = buttonNormalTex->getProperties().getWidth();
		customBtnStyle.height = buttonNormalTex->getProperties().getHeight();

		// Make the button use a custom font for text
		customBtnStyle.font = ExampleFramework::loadFont(ExampleFont::SegoeUILight, { 24 });
		customBtnStyle.fontSize = 24;

		// Offset the position of the text within the button, to match the texture
		customBtnStyle.contentOffset.top = 20;
		customBtnStyle.contentOffset.left = 15;
		customBtnStyle.contentOffset.right = 65;

		skin->setStyle("CustomButtonStyle", customBtnStyle);

		// Create the button that uses the custom style
		GUIButton* customButton = mainPanel->addNewElement<GUIButton>(HString("Click me!"), "CustomButtonStyle");
		customButton->setPosition(800, 50);

		// Add a header
		GUILabel* customButtonLbl = mainPanel->addNewElement<GUILabel>(HString("Custom button"), "HeaderLabelStyle");
		customButtonLbl->setPosition(800, 10);
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
	Application::startUp(videoMode, "Example", false);

	// Load a resource manifest so previously saved Fonts can find their child Texture resources
	ExampleFramework::loadResourceManifest();

	// Set up the GUI elements
	setUpGUI();

	// Save the manifest, in case we did any asset importing during the setup stage
	ExampleFramework::saveResourceManifest();

	// Runs the main loop that does most of the work. This method will exit when user closes the main
	// window or exits in some other way.
	Application::instance().runMainLoop();

	// When done, clean up
	Application::shutDown();

	return 0;
}