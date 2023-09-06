#pragma once

namespace Regal::Game
{
	class GameObject
	{
	public:
		GameObject(const char* objectName) : name(objectName) {};
		~GameObject() {};

		virtual void CreateResource() = 0;
		virtual void Initialize() = 0;
		virtual void Update(float elapsedTime) = 0;
		virtual void Render() = 0;
		virtual void DrawDebug() = 0;

	protected:
		//Imguiのウィンドウ起動するか
		bool activeDebugWindow = false;

		std::string name;
	};
}
