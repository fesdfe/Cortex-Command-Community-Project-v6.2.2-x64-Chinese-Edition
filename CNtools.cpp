#include "GUI.h"

// ��ȡ�������������а�������������������[ ��Ʒ�� ]���򷵻ظ���Ʒ�������򷵻�ԭ��
std::string CN_getNameFromDescription(std::string Description, std::string Name) {
	if (Description.length() > 2)
		if (Description.at(0) == '[') {
			if (Description.find(']') != -1) {
				Description = Description.substr(1, Description.find(']') - 1);
				return Description;
			}
		}
	return Name;
}