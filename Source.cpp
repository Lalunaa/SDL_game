#include <SDL.h>
#include <SDL_ttf.h>
#include <stdexcept>
#include <memory>
#include <string>
#include <set>
#include <tuple>
#include <iostream>
#include <cstdint>
#include <vector>
#include <array>
#include <SDL_image.h>

auto errthrow = [](const std::string &e) {
	std::string errstr = e + " : " + SDL_GetError();
	SDL_Quit();
	throw std::runtime_error(errstr);
};

std::shared_ptr<SDL_Window> init_window(const int width = 320, const int height = 200) { 
	if (SDL_Init(SDL_INIT_VIDEO) != 0) errthrow("SDL_Init");

	SDL_Window *win = SDL_CreateWindow("Leaping Sheep",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		width, height, SDL_WINDOW_SHOWN);
	if (win == nullptr) errthrow("SDL_CreateWindow");
	std::shared_ptr<SDL_Window> window(win, [](SDL_Window * ptr) {
		SDL_DestroyWindow(ptr); 
	});
	return window;
}

std::shared_ptr<SDL_Renderer> init_renderer(std::shared_ptr<SDL_Window> window) { 
	SDL_Renderer *ren = SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (ren == nullptr) errthrow("SDL_CreateRenderer");
	std::shared_ptr<SDL_Renderer> renderer(ren, [](SDL_Renderer * ptr) {
		SDL_DestroyRenderer(ptr);
	});
	return renderer;
}

std::shared_ptr<SDL_Texture> load_texture(const std::shared_ptr<SDL_Renderer> renderer, const std::string fname) {
	SDL_Surface *bmp = IMG_Load(fname.c_str());
	if (bmp == nullptr) errthrow("IMG_Load");
	std::shared_ptr<SDL_Surface> bitmap(bmp, [](SDL_Surface * ptr) {
		SDL_FreeSurface(ptr);
	});

	SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer.get(), bitmap.get());
	if (tex == nullptr) errthrow("SDL_CreateTextureFromSurface");
	std::shared_ptr<SDL_Texture> texture(tex, [](SDL_Texture * ptr) {
		SDL_DestroyTexture(ptr);
	});
	return texture;
}


using pos_t = std::array<double, 2>;  

pos_t operator +(const pos_t &a, const pos_t &b) { 
	return { a[0] + b[0], a[1] + b[1] };
}
pos_t operator -(const pos_t &a, const pos_t &b) {
	return { a[0] - b[0], a[1] - b[1] };
}
pos_t operator *(const pos_t &a, const pos_t &b) {
	return { a[0] * b[0], a[1] * b[1] };
}
pos_t operator *(const pos_t &a, const double &b) {
	return { a[0] * b, a[1] * b };
}

class player {  
public:
	pos_t position;
	pos_t velocity = { 0,0 };
	bool dead = false;
};

class obstacle {
public:
	pos_t position;
	pos_t velocity = { 0,0 };
};

bool checkCollision(SDL_Rect a, SDL_Rect b)
{
	int leftA, leftB;
	int rightA, rightB;
	int topA, topB;
	int bottomA, bottomB;

	leftA = a.x; 
	rightA = a.x + a.w;
	topA = a.y; 
	bottomA = a.y + a.h;

	leftB = b.x;
	rightB = b.x + b.w;
	topB = b.y;
	bottomB = b.y + b.h;

	if (bottomA <= topB)
	{
		return false;
	}

	if (topA >= bottomB)
	{
		return false;
	}

	if (rightA <= leftB)
	{
		return false;
	}

	if (leftA >= rightB)
	{
		return false;
	}

	return true;
}

int main(int argc, char **argv) {
	auto window = init_window(800, 600);
	auto renderer = init_renderer(window);

	TTF_Init();
	TTF_Font * font = TTF_OpenFont("Starjedi.ttf", 25);
	SDL_Color color = { 0, 0, 0 };
	SDL_Surface * surface = TTF_RenderText_Solid(font, " Time: ", color);
	SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer.get(), surface);

	auto background_texture = load_texture(renderer, "bg.png");
	auto ground_texture = load_texture(renderer, "ground.png");
	auto player_texture = load_texture(renderer, "sheep.png");
	auto obstacle_texture = load_texture(renderer, "hurdle.png");

	player player;
	player.position = { 50, 350 };

	obstacle obstacle;

	float gameTime = 0;
	float interval = 1;
	char data[50];

	double dt = 1 / 30.0; 

	for (bool game_active = true; game_active; ) {
		if (!player.dead) gameTime += dt;
		sprintf_s(data, "Time: %d", (int)gameTime); 
		surface = TTF_RenderText_Solid(font, data, color); 
		texture = SDL_CreateTextureFromSurface(renderer.get(), surface);

		if (gameTime > interval && gameTime <= 25 && !player.dead) {
			interval += rand() % 3 + 3;
			obstacle.position = pos_t{ 800, 350 };
			obstacle.velocity = pos_t{ -(double)(rand() % 6 + 10), 0 };
		}

		if (gameTime > interval && gameTime > 25 && gameTime <= 50 && !player.dead)
		{
			interval += rand() % 2 + 2;
			obstacle.position = pos_t{ 800, 350 };
			obstacle.velocity = pos_t{ -(double)(rand() % 6 + 14), 0 };
		}

		if (gameTime > interval && gameTime > 50 &&  gameTime <= 75 && !player.dead)
		{
			interval += rand() % 4 + 2;
			obstacle.position = pos_t{ 800, 350 };
			obstacle.velocity = pos_t{ -(double)(rand() % 4 + 20), 0 };
		}

		if (gameTime > interval && gameTime > 75 && !player.dead)
		{
			interval += rand() % 2 + 3;
			obstacle.position = pos_t{ 800, 350 };
			obstacle.velocity = pos_t{ -(double)(rand() % 4 + 22), 0 };
		}


		if (!player.dead) obstacle.position = obstacle.position + obstacle.velocity; 

		if (checkCollision({ (int)player.position[0], (int)player.position[1], 128, 128 }, { (int)obstacle.position[0], (int)obstacle.position[1], 128, 128 }) && !player.dead) { 
			player_texture = load_texture(renderer, "sheep_dead.png");
			player.dead = true;
		}

		SDL_Event event;
		while (SDL_PollEvent(&event)) { 
			if (event.type == SDL_QUIT) { game_active = false; }
		}

		const Uint8 *kstate = SDL_GetKeyboardState(NULL); 

		if (kstate[SDL_SCANCODE_SPACE] && player.position[1] >= 350 && !player.dead) { 
			player.position[1]--;
			player.velocity[1] = -22; 
		}


		if (player.position[1] < 350) {
			player.position = player.position + player.velocity;
			player.velocity = player.velocity + pos_t{ 0.0, 1 }; 
			if (player.position[1] > 350) {
				player.position[1] = 350;
			}
		}

		SDL_RenderClear(renderer.get());
		SDL_RenderCopy(renderer.get(), background_texture.get(), NULL, NULL);

		SDL_Rect dstrect = { (int)obstacle.position[0], (int)obstacle.position[1], 128, 128 }; 
		SDL_Point center = { 64, 64 }; 
		SDL_RenderCopyEx(renderer.get(), obstacle_texture.get(), NULL, &dstrect, 0, &center, SDL_FLIP_NONE);

		 dstrect = { (int)player.position[0], (int)player.position[1], 128, 128 }; 
		 center = { 64, 64 }; 
		SDL_RenderCopyEx(renderer.get(), player_texture.get(), NULL, &dstrect, 0, &center, SDL_FLIP_NONE); 

		dstrect = { 0, 475, 800, 125 }; 

		SDL_RenderCopyEx(renderer.get(), ground_texture.get(), NULL, &dstrect, 0, NULL, SDL_FLIP_NONE); 

		int texW = 0; 
		int texH = 0;
		SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
		dstrect = { 10, 0, texW, texH };
		SDL_RenderCopy(renderer.get(), texture, NULL, &dstrect);

		if (player.dead) {
			TTF_Font * font = TTF_OpenFont("Starjedi.ttf", 50);
			SDL_Color color = { 0, 0, 0 };
			SDL_Surface * surface = TTF_RenderText_Solid(font, "Game over !", color);
			SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer.get(), surface);
			int texW = 0;
			int texH = 0;
			SDL_QueryTexture(texture, NULL, NULL, &texW, &texH); 
			dstrect = { 400 - texW / 2, 200 - texH / 2, texW, texH };
			SDL_RenderCopy(renderer.get(), texture, NULL, &dstrect);
			SDL_DestroyTexture(texture);
			SDL_FreeSurface(surface);
		}

		SDL_RenderPresent(renderer.get()); 
		SDL_Delay((int)(dt / 1000));
	}

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);
	TTF_CloseFont(font);
	TTF_Quit();
	SDL_Quit();
	return 0;
}
