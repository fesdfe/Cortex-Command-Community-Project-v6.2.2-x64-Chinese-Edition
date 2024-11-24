#include "SettingsInputMappingWizardGUI.h"
#include "UInputMan.h"

#include "GUI.h"
#include "AllegroBitmap.h"
#include "GUICollectionBox.h"
#include "GUIButton.h"
#include "GUILabel.h"

using namespace RTE;

void SettingsInputMappingWizardGUI::Clear() {
	m_ConfiguringPlayer = Players::NoPlayer;
	m_ConfiguringPlayerScheme = nullptr;

	m_ConfiguringDevice = InputDevice::DEVICE_KEYB_ONLY;
	m_ConfiguringDeviceIsGamepad = false;
	m_ConfiguringGamepadType = GamepadType::DPad;
	m_ConfiguringGamepadIndex = -1;

	m_ConfiguringManually = false;
	m_ConfigFinished = false;
	m_ConfigStep = 0;
	m_ConfigStepChange = false;

	m_NewInputScheme.Reset();
	m_NewInputSchemeApplied = false;

	m_BlinkTimer.Reset();

	m_DPadDiagramBitmaps.clear();
	m_DualAnalogDSDiagramBitmaps.clear();
	m_DualAnalogXBDiagramBitmaps.clear();
}

SettingsInputMappingWizardGUI::SettingsInputMappingWizardGUI(GUIControlManager* parentControlManager) :
    m_GUIControlManager(parentControlManager) {
	Clear();

	m_InputWizardScreenBox = dynamic_cast<GUICollectionBox*>(m_GUIControlManager->GetControl("CollectionBoxInputMappingWizard"));
	m_InputWizardTitleLabel = dynamic_cast<GUILabel*>(m_GUIControlManager->GetControl("LabelPlayerInputMappingWizardTitle"));

	int dpadDiagramBitampCount = 13;
	ContentFile("Base.rte/GUIs/Controllers/D-Pad.png").GetAsAnimation(m_DPadDiagramBitmaps, dpadDiagramBitampCount, COLORCONV_8_TO_32);

	int analogDiagramBitmapCount = 21;
	ContentFile("Base.rte/GUIs/Controllers/DualAnalogDS.png").GetAsAnimation(m_DualAnalogDSDiagramBitmaps, analogDiagramBitmapCount, COLORCONV_8_TO_32);
	ContentFile("Base.rte/GUIs/Controllers/DualAnalogXB.png").GetAsAnimation(m_DualAnalogXBDiagramBitmaps, analogDiagramBitmapCount, COLORCONV_8_TO_32);

	CreateManualConfigScreen();
	CreatePresetSelectionScreen();
}

void SettingsInputMappingWizardGUI::CreateManualConfigScreen() {
	m_WizardManualConfigScreen.ManualConfigBox = dynamic_cast<GUICollectionBox*>(m_GUIControlManager->GetControl("CollectionBoxWizardManualConfig"));

	m_WizardManualConfigScreen.ConfigDeviceTypeLabel = dynamic_cast<GUILabel*>(m_GUIControlManager->GetControl("LabelConfigDeviceType"));
	m_WizardManualConfigScreen.ConfigStepDescriptionLabel = dynamic_cast<GUILabel*>(m_GUIControlManager->GetControl("LabelConfigInputDescription"));
	m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel = dynamic_cast<GUILabel*>(m_GUIControlManager->GetControl("LabelRecommendedKeyInput"));
	m_WizardManualConfigScreen.GamepadConfigRecommendedBox = dynamic_cast<GUICollectionBox*>(m_GUIControlManager->GetControl("CollectionBoxGamepadRecommendedInput"));
	m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel = dynamic_cast<GUILabel*>(m_GUIControlManager->GetControl("LabelRecommendedGamepadInput"));
	m_WizardManualConfigScreen.GamepadConfigRecommendedDiagramBox = dynamic_cast<GUICollectionBox*>(m_GUIControlManager->GetControl("CollectionBoxRecommendedGamepadInputDiagram"));

	m_WizardManualConfigScreen.ConfigStepLabel = dynamic_cast<GUILabel*>(m_GUIControlManager->GetControl("LabelConfigStep"));
	m_WizardManualConfigScreen.PrevConfigStepButton = dynamic_cast<GUIButton*>(m_GUIControlManager->GetControl("ButtonConfigPrevStep"));
	m_WizardManualConfigScreen.NextConfigStepButton = dynamic_cast<GUIButton*>(m_GUIControlManager->GetControl("ButtonConfigNextStep"));
	m_WizardManualConfigScreen.ResetConfigButton = dynamic_cast<GUIButton*>(m_GUIControlManager->GetControl("ButtonConfigRestart"));
	m_WizardManualConfigScreen.DiscardOrApplyConfigButton = dynamic_cast<GUIButton*>(m_GUIControlManager->GetControl("ButtonDiscardOrApply"));
}

void SettingsInputMappingWizardGUI::CreatePresetSelectionScreen() {
	m_WizardPresetSelectScreen.PresetSelectBox = dynamic_cast<GUICollectionBox*>(m_GUIControlManager->GetControl("CollectionBoxWizardPresets"));
	m_WizardPresetSelectScreen.CloseWizardButton = dynamic_cast<GUIButton*>(m_GUIControlManager->GetControl("ButtonCloseWizardBox"));

	m_WizardPresetSelectScreen.PresetSelectSNESButton = dynamic_cast<GUIButton*>(m_GUIControlManager->GetControl("ButtonPresetDPadSNES"));
	m_WizardPresetSelectScreen.PresetSelectDS4Button = dynamic_cast<GUIButton*>(m_GUIControlManager->GetControl("ButtonPresetAnalogDS4"));
	m_WizardPresetSelectScreen.PresetSelectXB360Button = dynamic_cast<GUIButton*>(m_GUIControlManager->GetControl("ButtonPresetAnalogXB360"));
	m_WizardPresetSelectScreen.StartConfigDPadTypeButton = dynamic_cast<GUIButton*>(m_GUIControlManager->GetControl("ButtonConfigDPadType"));
	m_WizardPresetSelectScreen.StartConfigAnalogDSTypeButton = dynamic_cast<GUIButton*>(m_GUIControlManager->GetControl("ButtonConfigAnalogTypeDS"));
	m_WizardPresetSelectScreen.StartConfigAnalogXBTypeButton = dynamic_cast<GUIButton*>(m_GUIControlManager->GetControl("ButtonConfigAnalogTypeXB"));

	// Ownership of the AllegroBitmaps is passed to the GUIControlManager.
	dynamic_cast<GUICollectionBox*>(m_GUIControlManager->GetControl("CollectionBoxConfigDPadTypeDiagram"))->SetDrawImage(new AllegroBitmap(m_DPadDiagramBitmaps[0]));
	dynamic_cast<GUICollectionBox*>(m_GUIControlManager->GetControl("CollectionBoxPresetDPadSNESDiagram"))->SetDrawImage(new AllegroBitmap(m_DPadDiagramBitmaps[0]));
	dynamic_cast<GUICollectionBox*>(m_GUIControlManager->GetControl("CollectionBoxConfigAnalogTypeDSDiagram"))->SetDrawImage(new AllegroBitmap(m_DualAnalogDSDiagramBitmaps[0]));
	dynamic_cast<GUICollectionBox*>(m_GUIControlManager->GetControl("CollectionBoxPresetAnalogDS4Diagram"))->SetDrawImage(new AllegroBitmap(m_DualAnalogDSDiagramBitmaps[0]));
	dynamic_cast<GUICollectionBox*>(m_GUIControlManager->GetControl("CollectionBoxConfigAnalogTypeXBDiagram"))->SetDrawImage(new AllegroBitmap(m_DualAnalogXBDiagramBitmaps[0]));
	dynamic_cast<GUICollectionBox*>(m_GUIControlManager->GetControl("CollectionBoxPresetAnalogXB360Diagram"))->SetDrawImage(new AllegroBitmap(m_DualAnalogXBDiagramBitmaps[0]));
}

bool SettingsInputMappingWizardGUI::IsEnabled() const {
	return m_InputWizardScreenBox->GetVisible() && m_InputWizardScreenBox->GetEnabled();
}

void SettingsInputMappingWizardGUI::SetEnabled(bool enable, int player, InputScheme* playerScheme) {
	m_InputWizardScreenBox->SetVisible(enable);
	m_InputWizardScreenBox->SetEnabled(enable);

	if (enable) {
		m_InputWizardTitleLabel->SetText("玩 家  " + std::to_string(player + 1) + "  输 入 设 置"); //"P L A Y E R   "  "   I N P U T   C O N F I G U R A T I O N"

		m_ConfiguringPlayer = static_cast<Players>(player);
		m_ConfiguringPlayerScheme = playerScheme;
		m_ConfiguringDevice = m_ConfiguringPlayerScheme->GetDevice();

		if (m_ConfiguringDevice == InputDevice::DEVICE_KEYB_ONLY || m_ConfiguringDevice == InputDevice::DEVICE_MOUSE_KEYB) {
			m_ConfiguringDeviceIsGamepad = false;
			m_ConfiguringGamepadIndex = -1;
			ShowManualConfigScreen();
		} else {
			m_ConfiguringDeviceIsGamepad = true;
			m_ConfiguringGamepadIndex = g_UInputMan.GetJoystickIndex(m_ConfiguringDevice);
			ShowPresetSelectionScreen();
		}
	} else {
		m_ConfiguringPlayer = Players::NoPlayer;
		m_ConfiguringPlayerScheme = nullptr;
		m_ConfigFinished = false;
		m_NewInputSchemeApplied = false;

		g_UInputMan.SetSkipHandlingSpecialInput(false);
	}
}

GUICollectionBox* SettingsInputMappingWizardGUI::GetActiveDialogBox() const {
	return (m_InputWizardScreenBox->GetEnabled() && m_InputWizardScreenBox->GetVisible()) ? m_InputWizardScreenBox : nullptr;
}

bool SettingsInputMappingWizardGUI::IsConfiguringManually() const {
	return m_ConfiguringManually && m_WizardManualConfigScreen.ManualConfigBox->GetVisible() && m_WizardManualConfigScreen.ManualConfigBox->GetEnabled();
}

void SettingsInputMappingWizardGUI::ShowManualConfigScreen() {
	m_WizardPresetSelectScreen.PresetSelectBox->SetVisible(false);
	m_WizardPresetSelectScreen.PresetSelectBox->SetEnabled(false);

	m_WizardManualConfigScreen.ManualConfigBox->SetVisible(true);
	m_WizardManualConfigScreen.ManualConfigBox->SetEnabled(true);
	m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetVisible(!m_ConfiguringDeviceIsGamepad);
	m_WizardManualConfigScreen.GamepadConfigRecommendedBox->SetVisible(m_ConfiguringDeviceIsGamepad);

	std::string inputDeviceName;
	if (m_ConfiguringDeviceIsGamepad) {
		switch (m_ConfiguringGamepadType) {
			case SettingsInputMappingWizardGUI::DPad:
				inputDeviceName = "Gamepad (Classic D-Pad Style)";
				m_WizardManualConfigScreen.GamepadConfigRecommendedDiagramBox->Resize(m_DPadDiagramBitmaps[0]->w, m_DPadDiagramBitmaps[0]->h);
				break;
			case SettingsInputMappingWizardGUI::AnalogDualShock:
				inputDeviceName = "Dual Analog Gamepad (DualShock Style)";
				m_WizardManualConfigScreen.GamepadConfigRecommendedDiagramBox->Resize(m_DualAnalogDSDiagramBitmaps[0]->w, m_DualAnalogDSDiagramBitmaps[0]->h);
				break;
			case SettingsInputMappingWizardGUI::AnalogXbox:
				inputDeviceName = "Dual Analog Gamepad (Xbox Style)";
				m_WizardManualConfigScreen.GamepadConfigRecommendedDiagramBox->Resize(m_DualAnalogXBDiagramBitmaps[0]->w, m_DualAnalogXBDiagramBitmaps[0]->h);
				break;
			default:
				RTEAbort("Invalid GamepadType passed to SettingsInputMappingWizardGUI::ShowManualConfigScreen!");
				break;
		}
		m_WizardManualConfigScreen.GamepadConfigRecommendedDiagramBox->CenterInParent(true, true);
	} else {
		if (m_ConfiguringDevice == InputDevice::DEVICE_KEYB_ONLY) {
			inputDeviceName = "仅键盘"; //Keyboard Only
		} else if (m_ConfiguringDevice == InputDevice::DEVICE_MOUSE_KEYB) {
			inputDeviceName = "键鼠"; //Mouse + Keyboard
		}
	}
	m_WizardManualConfigScreen.ConfigDeviceTypeLabel->SetText(inputDeviceName);

	g_UInputMan.SetSkipHandlingSpecialInput(true);

	ResetManualConfigSequence();
	m_ConfiguringManually = true;
}

void SettingsInputMappingWizardGUI::ShowPresetSelectionScreen() {
	m_WizardManualConfigScreen.ManualConfigBox->SetVisible(false);
	m_WizardManualConfigScreen.ManualConfigBox->SetEnabled(false);

	m_WizardPresetSelectScreen.PresetSelectBox->SetVisible(true);
	m_WizardPresetSelectScreen.PresetSelectBox->SetEnabled(true);
}

void SettingsInputMappingWizardGUI::ResetManualConfigSequence() {
	m_ConfigFinished = false;
	m_ConfigStep = 0;
	m_ConfigStepChange = true;
	m_NewInputScheme.Reset();
	m_NewInputScheme.SetDevice(m_ConfiguringDevice);
}

void SettingsInputMappingWizardGUI::ApplyGamepadInputPreset(GamepadType gamepadType) {
	switch (gamepadType) {
		case GamepadType::DPad:
			m_ConfiguringPlayerScheme->SetPreset(InputScheme::InputPreset::PresetGamepadSNES);
			break;
		case GamepadType::AnalogDualShock:
			m_ConfiguringPlayerScheme->SetPreset(InputScheme::InputPreset::PresetGamepadDS4);
			break;
		case GamepadType::AnalogXbox:
			m_ConfiguringPlayerScheme->SetPreset(InputScheme::InputPreset::PresetGamepadXbox360);
			break;
		default:
			RTEAbort("Invalid GamepadType passed to SettingsInputMappingWizardGUI::ApplyGamepadInputPreset!");
			break;
	}
	g_GUISound.ButtonPressSound()->Play();
	m_NewInputSchemeApplied = true;
}

void SettingsInputMappingWizardGUI::ApplyManuallyConfiguredScheme() {
	m_ConfiguringPlayerScheme->SetDevice(m_NewInputScheme.GetDevice());
	m_ConfiguringPlayerScheme->SetPreset(InputScheme::InputPreset::NoPreset);

	// Hard map the mouse controls and set names so they don't show as undefined after applying the new scheme.
	if (m_ConfiguringDevice == InputDevice::DEVICE_MOUSE_KEYB) {
		m_NewInputScheme.GetInputMappings()->at(InputElements::INPUT_FIRE).SetMouseButton(MouseButtons::MOUSE_LEFT);
		m_NewInputScheme.GetInputMappings()->at(InputElements::INPUT_PIEMENU_ANALOG).SetMouseButton(MouseButtons::MOUSE_RIGHT);
		m_NewInputScheme.GetInputMappings()->at(InputElements::INPUT_AIM).SetPresetDescription("Mouse Move");
		m_NewInputScheme.GetInputMappings()->at(InputElements::INPUT_AIM_UP).SetPresetDescription("Mouse Move");
		m_NewInputScheme.GetInputMappings()->at(InputElements::INPUT_AIM_DOWN).SetPresetDescription("Mouse Move");
		m_NewInputScheme.GetInputMappings()->at(InputElements::INPUT_AIM_LEFT).SetPresetDescription("Mouse Move");
		m_NewInputScheme.GetInputMappings()->at(InputElements::INPUT_AIM_RIGHT).SetPresetDescription("Mouse Move");
	}
	std::swap(*m_ConfiguringPlayerScheme->GetInputMappings(), *m_NewInputScheme.GetInputMappings());

	m_NewInputSchemeApplied = true;
	m_ConfiguringManually = false;
}

bool SettingsInputMappingWizardGUI::HandleInputEvents(GUIEvent& guiEvent) {
	if (m_WizardManualConfigScreen.ManualConfigBox->GetVisible()) {
		HandleManualConfigScreenInputEvents(guiEvent);
	} else if (m_WizardPresetSelectScreen.PresetSelectBox->GetVisible()) {
		HandlePresetSelectScreenInputEvents(guiEvent);
	}
	if (m_NewInputSchemeApplied) {
		SetEnabled(false);
		return true;
	}
	return false;
}

void SettingsInputMappingWizardGUI::HandleManualConfigScreenInputEvents(GUIEvent& guiEvent) {
	if (guiEvent.GetType() == GUIEvent::Command) {
		if (guiEvent.GetControl() == m_WizardManualConfigScreen.PrevConfigStepButton) {
			g_GUISound.ButtonPressSound()->Play();
			if (m_ConfigFinished) {
				m_ConfigFinished = false;
			} else {
				m_ConfigStep--;
			}
			m_ConfigStepChange = true;
		} else if (guiEvent.GetControl() == m_WizardManualConfigScreen.NextConfigStepButton) {
			g_GUISound.ButtonPressSound()->Play();
			m_ConfigStep++;
			m_ConfigStepChange = true;
		} else if (guiEvent.GetControl() == m_WizardManualConfigScreen.ResetConfigButton) {
			g_GUISound.ButtonPressSound()->Play();
			ResetManualConfigSequence();
		} else if (guiEvent.GetControl() == m_WizardManualConfigScreen.DiscardOrApplyConfigButton) {
			g_GUISound.ButtonPressSound()->Play();
			if (m_ConfigFinished) {
				ApplyManuallyConfiguredScheme();
			} else {
				if (m_ConfiguringDeviceIsGamepad) {
					ShowPresetSelectionScreen();
				} else {
					SetEnabled(false);
				}
			}
		}
	}
}

void SettingsInputMappingWizardGUI::HandlePresetSelectScreenInputEvents(GUIEvent& guiEvent) {
	if (guiEvent.GetType() == GUIEvent::Command) {
		if (guiEvent.GetControl() == m_WizardPresetSelectScreen.CloseWizardButton) {
			g_GUISound.ButtonPressSound()->Play();
			SetEnabled(false);
		} else if (guiEvent.GetControl() == m_WizardPresetSelectScreen.StartConfigDPadTypeButton) {
			g_GUISound.ButtonPressSound()->Play();
			m_ConfiguringGamepadType = GamepadType::DPad;
			ShowManualConfigScreen();
		} else if (guiEvent.GetControl() == m_WizardPresetSelectScreen.StartConfigAnalogDSTypeButton) {
			g_GUISound.ButtonPressSound()->Play();
			m_ConfiguringGamepadType = GamepadType::AnalogDualShock;
			ShowManualConfigScreen();
		} else if (guiEvent.GetControl() == m_WizardPresetSelectScreen.StartConfigAnalogXBTypeButton) {
			g_GUISound.ButtonPressSound()->Play();
			m_ConfiguringGamepadType = GamepadType::AnalogXbox;
			ShowManualConfigScreen();
		} else if (guiEvent.GetControl() == m_WizardPresetSelectScreen.PresetSelectSNESButton) {
			ApplyGamepadInputPreset(GamepadType::DPad);
		} else if (guiEvent.GetControl() == m_WizardPresetSelectScreen.PresetSelectDS4Button) {
			ApplyGamepadInputPreset(GamepadType::AnalogDualShock);
		} else if (guiEvent.GetControl() == m_WizardPresetSelectScreen.PresetSelectXB360Button) {
			ApplyGamepadInputPreset(GamepadType::AnalogXbox);
		}
	}
}

void SettingsInputMappingWizardGUI::HandleManualConfigSequence() {
	if (m_ConfigStepChange) {
		HandleManualConfigStepChange();
	}

	if (m_ConfigFinished) {
		m_GUIControlManager->GetManager()->SetFocus(m_BlinkTimer.AlternateReal(500) ? m_WizardManualConfigScreen.DiscardOrApplyConfigButton : nullptr);
	} else {
		bool inputCaptured = false;
		switch (m_ConfiguringDevice) {
			case InputDevice::DEVICE_KEYB_ONLY:
				inputCaptured = UpdateKeyboardConfigSequence();
				break;
			case InputDevice::DEVICE_MOUSE_KEYB:
				inputCaptured = UpdateMouseAndKeyboardConfigSequence();
				break;
			default:
				int diagramBitmapIndex = m_BlinkTimer.AlternateReal(500) ? m_ConfigStep + 1 : 0;
				if (m_ConfiguringGamepadType == GamepadType::DPad) {
					m_WizardManualConfigScreen.GamepadConfigRecommendedDiagramBox->SetDrawImage(new AllegroBitmap(m_DPadDiagramBitmaps.at(diagramBitmapIndex)));
					inputCaptured = UpdateGamepadDPadConfigSequence();
				} else {
					if (m_ConfiguringGamepadType == GamepadType::AnalogDualShock) {
						m_WizardManualConfigScreen.GamepadConfigRecommendedDiagramBox->SetDrawImage(new AllegroBitmap(m_DualAnalogDSDiagramBitmaps.at(diagramBitmapIndex)));
					} else {
						m_WizardManualConfigScreen.GamepadConfigRecommendedDiagramBox->SetDrawImage(new AllegroBitmap(m_DualAnalogXBDiagramBitmaps.at(diagramBitmapIndex)));
					}
					inputCaptured = UpdateGamepadAnalogConfigSequence();
				}
				break;
		}
		if (inputCaptured) {
			if (!m_ConfigFinished) {
				m_ConfigStep++;
			}
			g_GUISound.ExitMenuSound()->Play();
			m_ConfigStepChange = true;
		}
	}
}

void SettingsInputMappingWizardGUI::HandleManualConfigStepChange() {
	int configuringDeviceSteps = 0;

	switch (m_ConfiguringDevice) {
		case InputDevice::DEVICE_KEYB_ONLY:
			configuringDeviceSteps = m_KeyboardConfigSteps;
			break;
		case InputDevice::DEVICE_MOUSE_KEYB:
			configuringDeviceSteps = m_MouseAndKeyboardConfigSteps;
			break;
		case InputDevice::DEVICE_GAMEPAD_1:
		case InputDevice::DEVICE_GAMEPAD_2:
		case InputDevice::DEVICE_GAMEPAD_3:
		case InputDevice::DEVICE_GAMEPAD_4:
			configuringDeviceSteps = (m_ConfiguringGamepadType == GamepadType::DPad) ? m_DPadConfigSteps : m_DualAnalogConfigSteps;
			break;
		default:
			RTEAbort("Invalid InputDevice passed to SettingsInputMappingWizardGUI::HandleManualConfigSequence!");
			break;
	}
	if (!m_ConfigFinished) {
		m_WizardManualConfigScreen.ConfigStepLabel->SetText("第 " + std::to_string(m_ConfigStep + 1) + " 步 , 共 " + std::to_string(configuringDeviceSteps)); //"Step "  " of "
		m_WizardManualConfigScreen.DiscardOrApplyConfigButton->SetText("放弃更改"); //Discard Changes
		m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetVisible(true);
		if (m_ConfiguringDeviceIsGamepad) {
			m_WizardManualConfigScreen.GamepadConfigRecommendedBox->SetVisible(true);
		} else {
			m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetVisible(true);
		}
	} else {
		m_WizardManualConfigScreen.ConfigStepLabel->SetText("设置完成"); //Config Complete
		m_WizardManualConfigScreen.DiscardOrApplyConfigButton->SetText("应用设置"); //Apply Changes
		m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetVisible(false);
		m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetVisible(false);
		m_WizardManualConfigScreen.GamepadConfigRecommendedBox->SetVisible(false);
		if (m_ConfigStep < configuringDeviceSteps - 1) {
			m_ConfigFinished = false;
		}
	}
	m_WizardManualConfigScreen.PrevConfigStepButton->SetVisible(m_ConfigStep != 0);
	m_WizardManualConfigScreen.NextConfigStepButton->SetVisible(m_ConfigStep < configuringDeviceSteps - 1);
}

bool SettingsInputMappingWizardGUI::UpdateKeyboardConfigSequence() {
	switch (m_ConfigStep) {
		case 0:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("移动或向上瞄准"); //MOVE or AIM UP
				m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[W] 或 [上方向键]"); //[W] or [Up Arrow]
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_AIM_UP)) {
				m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_L_UP);
				return true;
			}
			break;
		case 1:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("移动或向下瞄准"); //MOVE or AIM DOWN
				m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[S] 或 [下方向键]"); //[S] or [Down Arrow]
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_AIM_DOWN)) {
				m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_L_DOWN);
				return true;
			}
			break;
		case 2:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("向左移动"); //MOVE LEFT
				m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[A] 或 [左方向键]"); //[A] or [Left Arrow]
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_L_LEFT)) {
				return true;
			}
			break;
		case 3:
			m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("向右移动"); //MOVE RIGHT
			m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[D] 或 [右方向键]"); //[D] or [Right Arrow]
			m_ConfigStepChange = false;
			if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_L_RIGHT)) {
				return true;
			}
			break;
		case 4:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("开火或互动"); //FIRE or ACTIVATE
				m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[H] 或 [Num 1]"); //[H] or [Num 1]
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_FIRE)) {
				return true;
			}
			break;
		case 5:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("精确瞄准"); //SHARP AIM
				m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[J] 或 [Num 2]");//[J] or [Num 2]
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_AIM)) {
				return true;
			}
			break;
		case 6:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("环形菜单"); //PIE MENU
				m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[K] 或 [Num 3]"); //[K] or [Num 3]
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_PIEMENU_DIGITAL)) {
				return true;
			}
			break;
		case 7:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("跳跃"); //JUMP
				m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[L] 或 [Num Enter]"); //[L] or [Num Enter]
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_JUMP)) {
				return true;
			}
			break;
		case 8:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("下蹲"); //CROUCH
				m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[C] 或 [Num 0]"); //[C] or [Num 0]
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_CROUCH)) {
				return true;
			}
			break;
		case 9:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("上一个控制体"); //PREVIOUS BODY
				m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[U] 或 [Num 4]"); //[U] or [Num 4]
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_PREV)) {
				return true;
			}
			break;
		case 10:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("下一个控制体"); //NEXT BODY
				m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[Y] 或 [Num 5]"); //[Y] or [Num 5]
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_NEXT)) {
				return true;
			}
			break;
		case 11:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("武器重装弹"); //RELOAD WEAPON
				m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[R] 或 [Num Del.]"); //[R] or [Num Del.]
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_WEAPON_RELOAD)) {
				return true;
			}
			break;
		case 12:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("捡拾"); //PICK UP DEVICE
				m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[F] 或 [Num 6]"); //[F] or [Num 6]
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_WEAPON_PICKUP)) {
				return true;
			}
			break;
		case 13:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("丢弃"); //DROP DEVICE
				m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[B] 或 [Num 9]"); //[B] or [Num 9]
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_WEAPON_DROP)) {
				return true;
			}
			break;
		case 14:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("前一个设备"); //PREVIOUS DEVICE
				m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[Q] 或 [Num 7]"); //[Q] or [Num 7]
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_WEAPON_CHANGE_PREV)) {
				return true;
			}
			break;
		case 15:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("下一个设备"); //NEXT DEVICE
				m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[E] 或 [Num 8]"); //[E] or [Num 8]
				m_ConfigStepChange = false;
			}
			if (!m_ConfigFinished && m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_WEAPON_CHANGE_NEXT)) {
				m_ConfigFinished = true;
				return true;
			}
			break;
		default:
			RTEAbort("Trying to capture input for an invalid configuration step in SettingsInputMappingWizardGUI::UpdateKeyboardConfigSequence!");
			break;
	}
	return false;
}

bool SettingsInputMappingWizardGUI::UpdateMouseAndKeyboardConfigSequence() {
	switch (m_ConfigStep) {
		case 0:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("向上移动或跳跃"); //MOVE UP or JUMP
				m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[W]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_L_UP)) {
				m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_JUMP);
				return true;
			}
			break;
		case 1:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("向下移动或下蹲"); //MOVE DOWN or CROUCH
				m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[S]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_L_DOWN)) {
				m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_CROUCH);
				return true;
			}
			break;
		case 2:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("向左移动"); // MOVE LEFT
				m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[A]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_L_LEFT)) {
				return true;
			}
			break;
		case 3:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("向右移动"); // MOVE RIGHT
				m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[D]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_L_RIGHT)) {
				return true;
			}
			break;
		case 4:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("上一个控制体"); // PREVIOUS BODY
				m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[Q]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_PREV)) {
				return true;
			}
			break;
		case 5:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("下一个控制体"); // NEXT BODY
				m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[E]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_NEXT)) {
				return true;
			}
			break;
		case 6:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("武器重装弹"); // RELOAD WEAPON
				m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[R]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_WEAPON_RELOAD)) {
				return true;
			}
			break;
		case 7:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("捡拾"); // PICK UP DEVICE
				m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[F]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_WEAPON_PICKUP)) {
				return true;
			}
			break;
		case 8:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("丢弃"); // DROP DEVICE
				m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[G]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_WEAPON_DROP)) {
				return true;
			}
			break;
		case 9:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("前一个设备"); // PREVIOUS DEVICE
				m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[1]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_WEAPON_CHANGE_PREV)) {
				return true;
			}
			break;
		case 10:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("下一个设备"); // NEXT DEVICE
				m_WizardManualConfigScreen.ConfigStepRecommendedKeyLabel->SetText("[2]");
				m_ConfigStepChange = false;
			}
			if (!m_ConfigFinished && m_NewInputScheme.CaptureKeyMapping(InputElements::INPUT_WEAPON_CHANGE_NEXT)) {
				m_ConfigFinished = true;
				return true;
			}
			break;
		default:
			RTEAbort("Trying to capture input for an invalid configuration step in SettingsInputMappingWizardGUI::UpdateMouseAndKeyboardConfigSequence!");
			break;
	}
	return false;
}

bool SettingsInputMappingWizardGUI::UpdateGamepadDPadConfigSequence() {
	switch (m_ConfigStep) {
		case 0:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("移动或向上瞄准"); // MOVE or AIM UP
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[D-Pad Up]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_AIM_UP)) {
				m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_L_UP);
				return true;
			}
			break;
		case 1:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("移动或向下瞄准"); // MOVE or AIM DOWN
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[D-Pad Down]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_AIM_DOWN)) {
				m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_L_DOWN);
				return true;
			}
			break;
		case 2:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("向左移动"); // MOVE LEFT
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[D-Pad Left]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_L_LEFT)) {
				return true;
			}
			break;
		case 3:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("向右移动"); // MOVE RIGHT
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[D-Pad Right]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_L_RIGHT)) {
				return true;
			}
			break;
		case 4:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("开火或互动"); // FIRE or ACTIVATE
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[B Button]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_FIRE)) {
				return true;
			}
			break;
		case 5:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("精确瞄准"); // SHARP AIM
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[Y Button]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_AIM)) {
				return true;
			}
			break;
		case 6:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("指令菜单"); //COMMAND MENU
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[X Button]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_PIEMENU_DIGITAL)) {
				return true;
			}
			break;
		case 7:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("跳跃"); // JUMP
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[A Button]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_JUMP)) {
				return true;
			}
			break;
		case 8:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("下一个控制体"); // NEXT BODY
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[R. Bumper Button]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_NEXT)) {
				return true;
			}
			break;
		case 9:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("上一个控制体"); // PREVIOUS BODY
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[L. Bumper Button]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_PREV)) {
				return true;
			}
			break;
		case 10:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("START / PAUSE");
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[Start Button]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_START)) {
				return true;
			}
			break;
		case 11:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("重置活动"); //RESET ACTIVITY
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[Select Button]");
				m_ConfigStepChange = false;
			}
			if (!m_ConfigFinished && m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_BACK)) {
				m_ConfigFinished = true;
				return true;
			}
			break;
		default:
			RTEAbort("Trying to capture input for an invalid configuration step in SettingsInputMappingWizardGUI::UpdateGamepadDPadConfigSequence!");
			break;
	}
	return false;
}

bool SettingsInputMappingWizardGUI::UpdateGamepadAnalogConfigSequence() {
	switch (m_ConfigStep) {
		case 0:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("向上移动或跳跃"); //MOVE UP or JUMP
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[L. Stick Up]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_L_UP)) {
				m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_JUMP);
				return true;
			}
			break;
		case 1:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("向下移动或下蹲"); //MOVE DOWN or CROUCH
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[L. Stick Down]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_L_DOWN)) {
				m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_CROUCH);
				return true;
			}
			break;
		case 2:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("向左移动"); // MOVE LEFT
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[L. Stick Left]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_L_LEFT)) {
				return true;
			}
			break;
		case 3:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("向右移动"); // MOVE RIGHT
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[L. Stick Right]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_L_RIGHT)) {
				return true;
			}
			break;
		case 4:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("向上瞄准"); //ANALOG AIM UP
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[R. Stick Up]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_R_UP)) {
				return true;
			}
			break;
		case 5:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("向下瞄准"); //ANALOG AIM DOWN
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[R. Stick Down]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_R_DOWN)) {
				return true;
			}
			break;
		case 6:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("向左瞄准"); //ANALOG AIM LEFT
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[R. Stick Left]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_R_LEFT)) {
				return true;
			}
			break;
		case 7:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("向右瞄准"); //ANALOG AIM RIGHT
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[R. Stick Right]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_R_RIGHT)) {
				return true;
			}
			break;
		case 8:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("开火或互动"); //FIRE / ACTIVATE
				std::string buttonName = (m_ConfiguringGamepadType == GamepadType::AnalogDualShock) ? "[X Button]" : "[A Button]";
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText(buttonName + " 或 [RT]"); // or [R. Trigger]
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_FIRE)) {
				return true;
			}
			break;
		case 9:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("精确瞄准"); // SHARP AIM
				std::string buttonName = (m_ConfiguringGamepadType == GamepadType::AnalogDualShock) ? "[Square Button]" : "[X Button]";
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText(buttonName + " 或" + ((m_ConfiguringGamepadType == GamepadType::AnalogDualShock) ? "\n" : " ") + "[LT]"); //" or"  "[L. Trigger]"
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_AIM)) {
				return true;
			}
			break;
		case 10:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("指令菜单"); //COMMAND MENU
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText((m_ConfiguringGamepadType == GamepadType::AnalogDualShock) ? "[Circle Button]" : "[B Button]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, m_ConfiguringGamepadType == GamepadType::AnalogDualShock ? InputElements::INPUT_PIEMENU_ANALOG : InputElements::INPUT_PIEMENU_DIGITAL)) {
				return true;
			}
			break;
		case 11:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("下一个控制体"); // NEXT BODY
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[RB]"); //[R. Bumper Button]
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_NEXT)) {
				return true;
			}
			break;
		case 12:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("上一个控制体"); // PREVIOUS BODY
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[LB]"); //[L. Bumper Button]
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_PREV)) {
				return true;
			}
			break;
		case 13:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("武器重装弹"); // RELOAD WEAPON
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText((m_ConfiguringGamepadType == GamepadType::AnalogDualShock) ? "[Triangle Button]" : "[Y Button]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_WEAPON_RELOAD)) {
				return true;
			}
			break;
		case 14:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("捡拾"); // PICK UP DEVICE
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[D-Pad Up]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_WEAPON_PICKUP)) {
				return true;
			}
			break;
		case 15:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("丢弃"); // DROP DEVICE
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[D-Pad Down]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_WEAPON_DROP)) {
				return true;
			}
			break;
		case 16:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("下一个设备"); // NEXT DEVICE
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[D-Pad Right]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_WEAPON_CHANGE_NEXT)) {
				return true;
			}
			break;
		case 17:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("前一个设备"); // PREVIOUS DEVICE
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText("[D-Pad Left]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_WEAPON_CHANGE_PREV)) {
				return true;
			}
			break;
		case 18:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("START / PAUSE");
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText((m_ConfiguringGamepadType == GamepadType::AnalogDualShock) ? "[Options Button]" : "[Start Button]");
				m_ConfigStepChange = false;
			}
			if (m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_START)) {
				return true;
			}
			break;
		case 19:
			if (m_ConfigStepChange) {
				m_WizardManualConfigScreen.ConfigStepDescriptionLabel->SetText("重置活动"); //RESET ACTIVITY
				m_WizardManualConfigScreen.GamepadConfigStepRecommendedInputLabel->SetText((m_ConfiguringGamepadType == GamepadType::AnalogDualShock) ? "[Share Button]" : "[Back Button]");
				m_ConfigStepChange = false;
			}
			if (!m_ConfigFinished && m_NewInputScheme.CaptureJoystickMapping(m_ConfiguringGamepadIndex, InputElements::INPUT_BACK)) {
				m_ConfigFinished = true;
				return true;
			}
			break;
		default:
			RTEAbort("Trying to capture input for an invalid configuration step in SettingsInputMappingWizardGUI::UpdateGamepadAnalogConfigSequence!");
			break;
	}
	return false;
}
