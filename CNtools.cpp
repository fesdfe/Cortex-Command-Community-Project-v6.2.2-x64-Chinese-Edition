#include "GUI.h"

// 读取描述，若描述中包含被中括号括起来的[ 物品名 ]，则返回该物品名，否则返回原名
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