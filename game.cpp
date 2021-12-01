#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

//-> Padding specify the min closeness between created entities.
//   This value is used only in entity creation.
#define PADDING 30
//---

//-> Cast to float macro
#define CAST_FLOAT(x) static_cast<float>(x)
//---

using namespace std;

//-> Enum for movements.
enum Direction {UP, DOWN, LEFT, RIGHT};
//---

//-> This array will be used in the collision check of entities.
typedef struct _entityArray {
	sf::Vector2f pos;
	sf::Vector2u size;
} entityArray;
//---

class Object {
protected:
	sf::RenderWindow *window;
	sf::Texture texture;
	sf::Sprite sprite;
	sf::Vector2u xy_offset; //This will be used to shrink textures.
public:
	void init(	sf::RenderWindow *const window,
				const string &texturePath,
				const sf::Vector2f &pos,
				const sf::Vector2u &xy_offset);
	sf::Vector2f getPosition();
	sf::Vector2u getSize();
	void setPosition(const sf::Vector2f &newPos);
	void paint();
};

class Sandbag : public Object {
};

class Barrel : public Object {
	bool isVisible;
public:
	Barrel();
	bool getVisible(void);
	void setVisible(const bool &visible);
	//Paint of this class different from the Object Class' paint() method. Apply additional isVisible check.
	void paint();
};

class Bullet : public Object {
	sf::Vector2f speedVector;
public:
	Bullet *next;
	Bullet *prev;
	Bullet();
	~Bullet();
	//Init of this class different from the Object Class' init() method. Takes additional speed parameter.
	void init(	sf::RenderWindow *const window,
				const string &texturePath,
				const sf::Vector2f &pos,
				const Direction &dir,
				const float &speed);
	void move(void);
};

class Player; //Added also here because of circular dependancy of BulletList and Player
class BulletList {
	sf::RenderWindow *window;
	string texturePath;
	Bullet *list; //Head of the linked list
	Player *owner; //Owner of the fired bullets.
public:
	BulletList();
	~BulletList();
	void init(	sf::RenderWindow *const window,
				const string &texturePath,
				Player *const owner);
	void add(	const sf::Vector2f &pos,
				const int &state,
				const float &speed); 
	void remove(Bullet *temp);
	void update(Player *const players,
				Barrel *const barrels,
				Sandbag *const sandbags,
				const int &np,
				const int &nb,
				const int &ns);
};

class Player : public Object {
	sf::Texture *textures;
	int state;
	int s;
	int oldDir; //To decide opposite direction movements in walk according to old direction of soldier.
	int score;
public:
	void init(	sf::RenderWindow *const window,
				const string &textBasePath,
				const int &numTextures,
				const sf::Vector2f &pos,
				const sf::Vector2u &xy_offset);
	void walk(	const float speed,
				const Direction &dir,
				Player *const players,
				Barrel *const barrels,
				Sandbag *const sandbags,
				const int &np,
				const int &nb,
				const int &ns);
	sf::Vector2u getSize(void); //Get texture size. This is different from the Object Class' getSize() method.
	//To fire bullets.
	void fire(BulletList *const list, const float &speed);
	//After being hit by a bullet, then soldier will reborn at rand coordinate.
	void reborn(Player *const players,
				Barrel *const barrels,
				Sandbag *const sandbags,
				const int &np,
				const int &nb,
				const int &ns);
	void incrementScore(void);
	int getScore(void);
	void paint(void);
};

class Game{
	float speed;
	int numBarrels;
	int numSandbags;
	int numPlayers;
	int width;
	int height;
	sf::RenderWindow *window;
	sf::Texture bgTexture;
	sf::Sprite bgSprite;
	sf::Font *font;
	sf::Text *text;
	Barrel *barrels;
	Sandbag *sandbags;
	Player *players;
	BulletList *bullets;
	//-> These methods are used for place the entities at the begining.
	bool entityCollisionCheck(entityArray *const entities, const unsigned int &lastEntIndex);
	sf::Vector2f getRandCoord(const sf::Vector2u &textureSize);
	//---
	void initBackGround(void);
	void initEntities(void);
	void initFontAndText(const string &fontPath, const int textSize);
	void initGameEnv(void);
	void drawBackground(void);
	void drawEntities(void);
	void drawText(void);
	void update(void);
public:
	Game(const float &speed, const int &w, const int &h, const int &nb, const int &ns, const int &np);
	~Game();
	void run2player(void); //This method will be used to start the 2 player shooter game.
};

//-> This function is used by 3 class, so it is not a method of them.
//   I tried to write it as if it is a macro. But I do not know what the compiler
//   will do.
inline bool isCollide(	const sf::Vector2f &pos1,
						const sf::Vector2u &size1,
						const sf::Vector2f &pos2,
						const sf::Vector2u &size2)
{
	unsigned int x_limit, y_limit; //holds limits for the collision check
	float x_diff, y_diff; //holds difference of two points for collision check
	//-> x_limit is the width of the leftmost entity's texture size.
	//   x_diff is the x coord of rightmost minus x coord of leftmost entity.
	if (pos1.x <= pos2.x) {
		x_diff = pos2.x - pos1.x;
		x_limit = size1.x;
	} else {
		x_diff = pos1.x - pos2.x;
		x_limit = size2.x;
	}
	//---
	if ( x_limit >= x_diff ) { //if x collide then check also situation of y
		//-> y_limit is the height of the uppermost entity's texture size.
		//   y_diff is the y coord of lowermost - y coord of uppermost entity.
		if (pos1.y <= pos2.y) {
			y_diff = pos2.y - pos1.y;
			y_limit = size1.y;
		} else {
			y_diff = pos1.y - pos2.y;
			y_limit = size2.y;
		}
		//---
		if ( y_limit >= y_diff ) { //both x and y is collide means entities collide.
			return 1;
		} else { //if x is coolide and y is not then entities dont collide.
			return 0;
		}
	} else { //if there is no collision in x coord means entities not collide.
		return 0;
	}
}
//---


//////////////////////////////////// Definitions of Object Class
void Object::init(	sf::RenderWindow *const window,
					const string &texturePath,
					const sf::Vector2f &pos,
					const sf::Vector2u &xy_offset)
{
	//-> Fill the class' attributes and load texture from file.
	//   And create the sprite.
	this->window = window;
	this->xy_offset = xy_offset;
	if (!texture.loadFromFile(texturePath)) {
		cout << "[ERROR] Object texture loading error." << endl;
	}
	sprite.setTexture(texture);
	sprite.setPosition(pos);
	//---
}

inline sf::Vector2f Object::getPosition()
{
	return sprite.getPosition();
}

//-> In this method, texture size of the object will be shrunk
//   according to the its xy_offset attribute.
inline sf::Vector2u Object::getSize()
{
	sf::Vector2u size = texture.getSize();
	size.x -= xy_offset.x;
	size.y -= xy_offset.y;
	return size;
}

inline void Object::setPosition(const sf::Vector2f &newPos)
{
	sprite.setPosition(newPos);
}

inline void Object::paint()
{
	window->draw(sprite);
}


//////////////////////////////////// Definitions of Sandbag Class
//Sandbag class is directly inherited from Object Class. There is no additional changes.


//////////////////////////////////// Definitions of Barrel Class
//Set as visible in construction.
Barrel::Barrel() : isVisible(1) {};

inline bool Barrel::getVisible(void) { return isVisible; };

inline void Barrel::setVisible(const bool &visible) { isVisible = visible; };

//This is the overrided paint method. Additionally check the barrel's visibility.
inline void Barrel::paint() { if (isVisible == 1) { window->draw(sprite); } };


//////////////////////////////////// Definitions of Bullet Class
//NULL is assigned to next and prev pointers in construction.
Bullet::Bullet() : next(NULL), prev(NULL) {}

//This is overrided init method. This method decides direction and position of the bullet.
void Bullet::init(	sf::RenderWindow *const window,
				const string &texturePath,
				const sf::Vector2f &pos,
				const Direction &dir,
				const float &speed)
{
	sf::Vector2u bulletSize;
	this->window = window;
	if (!texture.loadFromFile(texturePath)) {
		cout << "[ERROR] Bullet texture loading error." << endl;
	}
	bulletSize = texture.getSize();
	sprite.setTexture(texture);

	//-> Here, origin of the bullet sprite is assigned according to the
	//   rotation of the bullet. After the rotation, origin always is the
	//   left-top of the rotated sprite. AND ALSO, position of the bullet
	//   are adjusted so bullet texture looks like it is come from the
	//   gun.
	//   Normally xy_offset is used to shrink shadow effect etc. for
	//   collision check. But in bullet we use it to set rotated bullets
	//   size correctly.
	switch (dir) {
		case UP:
			speedVector = sf::Vector2f(0, -speed);
			sprite.setPosition(pos);
			sprite.move(30,-25);
			xy_offset.x = 0;
			xy_offset.y = 0;
			break;
		case DOWN:
			speedVector = sf::Vector2f(0, speed);
			sprite.setOrigin(bulletSize.x - 1, bulletSize.y - 1);
			sprite.rotate(180);
			sprite.setPosition(pos);
			sprite.move(0,60);
			xy_offset.x = 0;
			xy_offset.y = 0;
			break;
		case LEFT:
			speedVector = sf::Vector2f(-speed, 0);
			sprite.setOrigin(bulletSize.x - 1, 0);
			sprite.rotate(270);
			sprite.setPosition(pos);
			sprite.move(-40,10);
			xy_offset.x = -20;
			xy_offset.y = 20;
			break;
		case RIGHT:
			speedVector = sf::Vector2f(speed, 0);
			sprite.setOrigin(0, bulletSize.y - 1);
			sprite.rotate(90);
			sprite.setPosition(pos);
			sprite.move(55,45);
			xy_offset.x = -20;
			xy_offset.y = 20;
			break;
		default:
			break;
	}
	//---
}

//-> This destructor is very critical.
//   Removed bullet's next and prev nodes are changed according to the
//   its values.
Bullet::~Bullet() {
	if ( prev != NULL && next != NULL ) {
		prev->next = next;
		next->prev = prev;
	} else if ( prev == NULL && next != NULL ) {
		next->prev = NULL;
	} else if ( prev != NULL && next == NULL ) {
		prev->next = NULL;
	}
}
//---

void Bullet::move(void)
{
	sprite.move(speedVector);
}

//////////////////////////////////// Definitions of BulletList Class
//NULL is assigned to list pointer in construction.
BulletList::BulletList() : list(NULL) {}

//-> Destructor prevents memory leaks. It removes everything from the list.
BulletList::~BulletList() {
	if ( list != NULL ) {
		Bullet *temp = list;
		while ( list != NULL ) {
			list = temp->next;
			delete temp;
			temp = list;
		}
	}
}
//---

//Classic init.
void BulletList::init(sf::RenderWindow *const window, const string &texturePath, Player *const owner)
{
	this->window = window;
	this->texturePath = texturePath;
	this->owner = owner;
}

//-> This method add new bullet to the bullets' linked list.
//   It decides the bullet's position according to the state of soldier.
void BulletList::add(const sf::Vector2f &pos, const int &state, const float &speed)
{
	Direction dir;
	//-> Direction decision according to the state.
	switch (state) {
		case 0:
		case 7:
		case 8:
			dir = UP;
			break;
		case 2:
		case 9:
		case 10:
			dir = RIGHT;
			break;
		case 4:
		case 3:
		case 11:
			dir = DOWN;
			break;
		case 6:
		case 12:
		case 13:
			dir = LEFT;
			break;
		default:
			return;
	}
	//---

	//-> Linked list node addition.
	if ( list != NULL ) {
		Bullet *temp = list;
		while ( temp->next != NULL ) {
			temp = temp->next;
		}
		temp->next = new Bullet;
		temp->next->prev = temp;
		temp->next->init(window, texturePath, pos, dir, speed);
	} else {
		list = new Bullet;
		list->init(window, texturePath, pos, dir, speed);
	}
	//---
}
//---

//-> This method just removes given node and adjust the list pointer.
//   Removed node's next and prev pointers are changed in bullet's destructor.
void BulletList::remove(Bullet *temp)
{
	if ( temp != NULL ) {
		if ( temp->prev == NULL && temp->next == NULL ) {
			list = NULL;
		} else if ( temp->prev == NULL && temp->next != NULL ) {
			list = temp->next;
		}
		delete temp;
	}
}
//---

//-> This method first check the collision of the bullets in the list.
//   Then move bullets.
void BulletList::update(Player *const players,
						Barrel *const barrels,
						Sandbag *const sandbags,
						const int &np,
						const int &nb,
						const int &ns)
{
	Bullet *temp = list; //Holds the list
	Bullet *newTemp; //Holds the next node of the removed temp.
	while ( temp != NULL ) {
		//Get position and size of the bullet
		sf::Vector2f bulletPos = temp->getPosition();
		sf::Vector2u bulletSize = temp->getSize();

		//-> If there is a collision then test variable will be set.
		//   Bullet will be removed and while loop will be reset without further collision check.
		int test = 0;
		for ( int i = 0 ; i < ns ; i++ ) {
			//-> Collision with sandbag just removes bullet.
			if ( isCollide(bulletPos, bulletSize, sandbags[i].getPosition(), sandbags[i].getSize()) ) {
				newTemp = temp->next;
				remove(temp);
				temp = newTemp;
				test = 1;
				break;
			} else {
				continue;
			}
			//---
		}
		if ( test == 1 ) {
			continue;
		}

		for ( int i = 0 ; i < nb ; i++ ) {
			//Check whether barrel is visible or not.
			if ( barrels[i].getVisible() == 0 ) {
				continue;
			}
			//-> If there is a collision with barrel, then both barrel and bullet will be removed.
			if ( isCollide(bulletPos, bulletSize, barrels[i].getPosition(), barrels[i].getSize()) ) {
				barrels[i].setVisible(0);
				newTemp = temp->next;
				remove(temp);
				temp = newTemp;
				test = 1;
				break;
			} else {
				continue;
			}
			//---
		}
		if ( test == 1 ) {
			continue;
		}
		
		for ( int i = 0 ; i < np ; i++ ) {
			if ( (players+i) == owner ) { //To prevent check of the bulletlist owner.
				continue;
			}
			//-> If there is a collision with a player, then player will be born at random location
			//   and owner of the bullet get a point.
			if ( isCollide(bulletPos, bulletSize, players[i].getPosition(), players[i].getSize()) ) {
				players[i].reborn(players, barrels, sandbags, np, nb, ns);
				owner->incrementScore();
				newTemp = temp->next;
				remove(temp);
				temp = newTemp;
				test = 1;
				break;
			} else {
				continue;
			}
			//---
		}
		if ( test == 1 ) {
			continue;
		}
		//---

		//-> This if block prevent the bullet from go beyond the window limit.
		if ( (bulletPos.x < -CAST_FLOAT(bulletSize.x)) || //Left window limit
			 (bulletPos.y < -CAST_FLOAT(bulletSize.y)) || //Up window limit
			 (bulletPos.x > window->getSize().x) || //Right window limit
			 (bulletPos.y > window->getSize().y) // Bottom window limit
			 ) {
			newTemp = temp->next;
			remove(temp);
			temp = newTemp;
			continue;
		}
		//---
		temp->move();
		temp->paint();
		temp = temp->next;
	}
}

//////////////////////////////////// Definitions of Player Class
void Player::init(	sf::RenderWindow *const window,
				const string &textBasePath,
				const int &numTextures,
				const sf::Vector2f &pos,
				const sf::Vector2u &xy_offset)
{
	this->window = window;
	this->xy_offset = xy_offset;
	textures = new sf::Texture[numTextures];
	for (int i = 0 ; i < numTextures ; i++) {
		//-> to_string method in the if block convert the uint to str
		if ( !(textures+i)->loadFromFile(textBasePath + "soldier" + to_string(i) + ".png") ) {
			cout << "[ERROR] Soldier texture loading error." << endl;
		}
		//---
	}
	state = 0;
	s = 0;
	oldDir = -1; //Means init step
	score = 0;
	//-> We will use soldier0.png at the beginning
	sprite.setTexture(textures[state]);
	//---
	sprite.setOrigin(CAST_FLOAT(xy_offset.x) / 2, CAST_FLOAT(xy_offset.y) / 2);
	sprite.setPosition(pos);
}

inline void Player::fire(BulletList *const list, const float &speed)
{
	list->add(getPosition(), state, speed);
}

//-> This method moves the soldier to the random location.
void Player::reborn(Player *const players,
					Barrel *const barrels,
					Sandbag *const sandbags,
					const int &np,
					const int &nb,
					const int &ns)
{
	sf::Vector2u curSize = getSize();
	sf::Vector2u limits = window->getSize() - curSize;
	sf::Vector2f newPos;
	//-> Collision check loop.
	while ( 1 ) {
		//In this loop, if check variable is 1 then while loop is reset.
		int check = 0;
		newPos.x = rand() % limits.x;
		newPos.y = rand() % limits.y;
		for ( int i = 0 ; i < nb ; i++ ) {
			if ( barrels[i].getVisible() == 0 ) { //Check whether barrel is visible or not.
				continue;
			}
			if ( isCollide(newPos, curSize, barrels[i].getPosition(), barrels[i].getSize()) ) {
				check = 1;
				break;
			} else {
				continue;
			}
		}
		if ( check == 1 ) {
			continue;
		}
		
		for ( int i = 0 ; i < ns ; i++ ) {
			if ( isCollide(newPos, curSize, sandbags[i].getPosition(), sandbags[i].getSize()) ) {
				check = 1;
				break;
			} else {
				continue;
			}
		}
		if ( check == 1 ) {
			continue;
		}

		for ( int i = 0 ; i < np ; i++ ) {
			if ( (players+i) == this ) { //To prevent check of same soldier.
				continue;
			}
			if ( isCollide(newPos, curSize, players[i].getPosition(), players[i].getSize()) ) {
				check = 1;
				break;
			} else {
				continue;
			}
		}
		if ( check == 0 ) {
			break;
		}
	}
	//---
	this->setPosition(newPos);
	this->paint();
}

inline void Player::incrementScore(void) { score += 1; }

inline int Player::getScore(void) { return score; }

void Player::walk(	const float speed,
					const Direction &dir,
					Player *const players,
					Barrel *const barrels,
					Sandbag *const sandbags,
					const int &np,
					const int &nb,
					const int &ns)
{
	sf::Vector2f velocityVector(0,0);
	//-> As different from the given state, I implement a mechanism that used for the
	//   opposite side movements (Press UP key when soldier look at the DOWN etc.).
	//   In intermediate phases of the soldire (state 1,3,5,7) according to the
	//   movement direction, old direction is guessed and saved. For example if soldier moves
	//   state 1 to state 0 (UP) then old state will be state 2 (RIGHT) because it is probably the old direction.
	//   Then if DOWN key pressed when soldier look at the UP, user move according to the oldDir.
	switch (state) {
		case 0:
			switch (dir) {
				case RIGHT:
					state = 1;
					break;
				case LEFT:
					s = 1;
					state = 7;
					break;
				case DOWN:
					if ( oldDir == LEFT ) {
						s = 1;
						state = 7;
					} else if ( oldDir == RIGHT ) {
						state = 1;
					} else {
						state = 1; //To handle init value (-1) of oldDir.
					}
					break;
				case UP:
					if ( s == 0 ) {
						state = 7;
						s = 1;
						velocityVector.x += 0;
						velocityVector.y += -1 * speed;
					} else if ( s == 1 ) {
						state = 8;
						s = 0;
						velocityVector.x += 0;
						velocityVector.y += -1 * speed;
					}
					break;
			}
			break;
		case 1:
			switch (dir) {
				case UP:
				case LEFT:
					state = 0;
					oldDir = RIGHT;
					break;
				case DOWN:
				case RIGHT:
					state = 2;
					oldDir = UP;
					break;
			}
			break;
		case 2:
			switch (dir) {
				case UP:
					state = 1;
					break;
				case DOWN:
					s = 1;
					state = 3;
					break;
				case LEFT:
					if ( oldDir == UP ) {
						state = 1;
					} else if ( oldDir == DOWN ) {
						s = 1;
						state = 3;
					}
					break;
				case RIGHT:
					if ( s == 0 ) {
						s = 1;
						state = 10;
						velocityVector.x += speed;
						velocityVector.y += 0;
					} else if ( s == 1 ) {
						s = 0;
						state = 9;
						velocityVector.x += speed;
						velocityVector.y += 0;
					}
					break;
			}
			break;
		case 3:
			switch (dir) {
				case UP:
				case RIGHT:
					state = 2;
					oldDir = DOWN;
					break;
				case DOWN:
					velocityVector.x += 0;
					velocityVector.y += speed;
				case LEFT:
					state = 4;
					oldDir = RIGHT;
					break;
			}
			break;
		case 4:
			switch (dir) {
				case RIGHT:
					state = 3;
					s = 1;
					break;
				case LEFT:
					state = 5;
					break;
				case UP:
					if ( oldDir == RIGHT ) {
						state = 3;
						s = 1;
					} else if ( oldDir == LEFT ) {
						state = 5;
					}
					break;
				case DOWN:
					if ( s == 0 ) {
						state = 3;
						s = 1;
						velocityVector.x += 0;
						velocityVector.y += speed;
					} else if ( s == 1 ) {
						state = 11;
						s = 0;
						velocityVector.x += 0;
						velocityVector.y += speed;
					}
					break;
			}
			break;
		case 5:
			switch (dir) {
				case UP:
				case LEFT:
					state = 6;
					oldDir = DOWN;
					break;
				case DOWN:
				case RIGHT:
					state = 4;
					oldDir = LEFT;
					break;
			}
			break;
		case 6:
			switch (dir) {
				case UP:
					state = 7;
					s = 1;
					break;
				case DOWN:
					state = 5;
					break;
				case RIGHT:
					if ( oldDir == DOWN ) {
						state = 5;
					} else if ( oldDir == UP ) {
						state = 7;
						s = 1;
					}
					break;
				case LEFT:
					if ( s == 0 ) {
						state = 13;
						s = 1;
						velocityVector.x += -1 * speed;
						velocityVector.y += 0;
					} else if ( s == 1 ) {
						state = 12;
						s = 0;
						velocityVector.x += -1 * speed;
						velocityVector.y += 0;
					}
					break;
			}
			break;
		case 7:
			switch (dir) {
				case UP:
					velocityVector.x += 0;
					velocityVector.y += -1 * speed;
				case RIGHT:
					state = 0;
					oldDir = LEFT;
					break;
				case DOWN:
				case LEFT:
					state = 6;
					oldDir = UP;
					break;
			}
			break;
		case 8:
			velocityVector.x += 0;
			velocityVector.y += -1 * speed;
			state = 0;
			break;
		case 9:
			velocityVector.x += speed;
			velocityVector.y += 0;
			state = 2;
			break;
		case 10:
			velocityVector.x += speed;
			velocityVector.y += 0;
			state = 2;
			break;
		case 11:
			velocityVector.x += 0;
			velocityVector.y += speed;
			state = 4;
			break;
		case 12:
			velocityVector.x += -1 * speed;
			velocityVector.y += 0;
			state = 6;
			break;
		case 13:
			velocityVector.x += -1 * speed;
			velocityVector.y += 0;
			state = 6;
			break;
		default:
			break;
	}
	//---

	sf::Vector2f newPos = getPosition() + velocityVector;
	sf::Vector2u curSize = getSize();
	//-> Collision check of the given soldier with barrels sandbags and other soldier(s).
	for ( int i = 0 ; i < nb ; i++ ) {
		if ( barrels[i].getVisible() == 0 ) {
			continue;
		}
		if ( isCollide(newPos, curSize, barrels[i].getPosition(), barrels[i].getSize()) ) {
			return;
		} else {
			continue;
		}
	}
	
	for ( int i = 0 ; i < ns ; i++ ) {
		if ( isCollide(newPos, curSize, sandbags[i].getPosition(), sandbags[i].getSize()) ) {
			return;
		} else {
			continue;
		}
	}

	for ( int i = 0 ; i < np ; i++ ) {
		if ( (players+i) == this ) { //To prevent check of same soldier.
			continue;
		}
		if ( isCollide(newPos, curSize, players[i].getPosition(), players[i].getSize()) ) {
			return;
		} else {
			continue;
		}
	}
	//---

	//-> This if block prevent the soldier from go beyond the window limit.
	if ( (newPos.x >= -(CAST_FLOAT(curSize.x)/2)) && //Left window limit
		 (newPos.y >= -(CAST_FLOAT(curSize.y)/2)) && //Up window limit
		 (newPos.x < window->getSize().x - (curSize.x)) && //Right window limit
		 (newPos.y < window->getSize().y - (curSize.y)) // Bottom window limit
		 ) {
		sprite.move(velocityVector); //If there is no collision then soldier will move
	}
	//---
}

inline sf::Vector2u Player::getSize()
{
	sf::Vector2u size = textures[state].getSize();
	size.x -= xy_offset.x;
	size.y -= xy_offset.y;
	return size;
}

inline void Player::paint() //Set state texture and paint user.
{
	sprite.setTexture(textures[state]);
	window->draw(sprite);
}

//////////////////////////////////// Definitions of Game Class
Game::Game(	const float &speed,
			const int &w,
			const int &h,
			const int &nb,
			const int &ns,
			const int &np)	:	speed(speed),
								numBarrels(nb),
								numSandbags(ns),
								numPlayers(np),
								width(w),
								height(h)
{
srand(time(NULL)); //Seed the random number generator.
}

Game::~Game() //Clear the memory.
{
	delete [] barrels;
	delete [] sandbags;
	delete [] players;
	delete [] bullets;
	delete text;
	delete font;
	delete window;
}

//-> 1 means there is a collision, 0 means no collision.
bool Game::entityCollisionCheck(entityArray *const entities, const unsigned int &lastEntIndex)
{
	//-> Compare the new entity location with older entities to prevent collision.
	sf::Vector2u extend(PADDING, PADDING);
	for (unsigned int i = 0 ; i < lastEntIndex ; i++) {
		if ( isCollide(entities[i].pos, entities[i].size + extend, entities[lastEntIndex].pos, entities[lastEntIndex].size + extend) ) {
			return 1;
		} else {
			continue;
		}
	}
	return 0;
}

inline void Game::initBackGround(void)
{
	if (!bgTexture.loadFromFile("textures/grass.png")) {
		cout << "[ERROR] Texture loading error." << endl;
		exit(1);
	}
	
	//-> If created sprite larger than texture, than repeat texture to fill sprite.
	bgTexture.setRepeated(true); 
	//---
	
	bgSprite.setTexture(bgTexture);
	
	//-> Sprite will be same width and height with window. In case of the larger
	//   sprite, then texture repeatedly fill the sprite area.
	bgSprite.setTextureRect(sf::IntRect(0, 0, width, height));
	//---
}

inline sf::Vector2f Game::getRandCoord(const sf::Vector2u &textureSize)
{
	//-> Create a random x and y according to the game width and height and also
	//   texture of the entities width and height.
	sf::Vector2f randCoord(
	randCoord.x = rand() % (width - textureSize.x),
	randCoord.y = rand() % (height - textureSize.y));
	//---
	return randCoord;
}

inline void Game::initEntities(void)
{
	//-> Memory allocations of entities.
	barrels = new Barrel[numBarrels];
	sandbags = new Sandbag[numSandbags];
	players = new Player[numPlayers];
	bullets = new BulletList[numPlayers];
	//---

	//-> This part created for the collision check.
	//   Size of entity texture and position of entity will be saved to the array.
	//   And also last saved item index will count.
	entityArray *entities = new entityArray[numBarrels + numSandbags + numPlayers];
	unsigned int lastEntIndex = 0;
	//---

	//-> Initialize the barrel, sandbag and player according to its numbers.
	//   In every step, firstly entity is initialized at (0,0) location, then
	//   texture size of this entity is saved to the "entites" array, than
	//   this item is moved to a random coordinate and also this coordinate
	//   is saved to the "entities" array. Then collision check is applied
	//   to the given "entities" array. lastEntIndex is the index of the
	//   new saved entitiy in the "entities" array.
	for (int i = 0 ; i < numBarrels ; i++ ) {
		(barrels+i)->init(window, "textures/barrel.png", sf::Vector2f(0,0), sf::Vector2u(5, 38));
		(entities+lastEntIndex)->size = (barrels+i)->getSize();
		do {
			(barrels+i)->setPosition(getRandCoord((barrels+i)->getSize()));
			(entities+lastEntIndex)->pos = (barrels+i)->getPosition();
		} while (entityCollisionCheck(entities, lastEntIndex));
		lastEntIndex++;
	}
	for (int i = 0 ; i < numSandbags ; i++ ) {
		(sandbags+i)->init(window, "textures/bags.png", sf::Vector2f(0,0), sf::Vector2u(5, 28)); 
		(entities+lastEntIndex)->size = (sandbags+i)->getSize();
		do {
			(sandbags+i)->setPosition(getRandCoord((sandbags+i)->getSize()));
			(entities+lastEntIndex)->pos = (sandbags+i)->getPosition();
		} while (entityCollisionCheck(entities, lastEntIndex));
		lastEntIndex++;
	}
	for (int i = 0 ; i < numPlayers ; i++ ) {
		(players+i)->init(window, "textures/", 14, sf::Vector2f(0,0), sf::Vector2u(50,50)); 
		(bullets+i)->init(window, "textures/bullet.png", (players+i));
		(entities+lastEntIndex)->size = (players+i)->getSize();
		do {
			(players+i)->setPosition(getRandCoord((players+i)->getSize()));
			(entities+lastEntIndex)->pos = (players+i)->getPosition();
		} while (entityCollisionCheck(entities, lastEntIndex));
		lastEntIndex++;
	}
	//---

	//-> "entites" array is not the attribute of the class, it is
	//   created in this method and so deleted in this method.
	delete [] entities;
	//---
}

inline void Game::initFontAndText(const string &fontPath, const int textSize)
{
	font = new sf::Font;
	text = new sf::Text;
	if (!font->loadFromFile(fontPath)) {
		cout << "[ERROR] Font loading error." << endl;
	}
	text->setFont(*font);
	text->setCharacterSize(textSize);
}

void Game::initGameEnv(void)
{
	window = new sf::RenderWindow(sf::VideoMode(width, height), "Shooter 2D");
	initBackGround();
	initEntities();
	initFontAndText("./font.ttf", 40);
}

inline void Game::drawEntities(void)
{
	for ( int i = 0 ; i < numBarrels ; i++ ) {
		barrels[i].paint();
	}
	for ( int i = 0 ; i < numSandbags ; i++ ) {
		sandbags[i].paint();
	}
	for ( int i = 0 ; i < numPlayers ; i++ ) {
		players[i].paint();
		bullets[i].update(players, barrels, sandbags, numPlayers, numBarrels, numSandbags);
	}
}

inline void Game::drawBackground(void) //Clear and draw.
{
	window->clear(sf::Color::Black);
	window->draw(bgSprite);
}

inline void Game::drawText(void)
{
	window->draw(*text);
}

inline void Game::update(void)
{
	drawBackground();
	drawEntities();
	drawText();
	window->display();
}

void Game::run2player(void)
{
	if ( numPlayers != 2 ) {
		cout << "[ERROR] This method should be run for 2 player games." << endl;
		return;
	}

	initGameEnv();

	sf::Event event;
	int pl1move = -1, pl2move = -1; //Holds movement dir of players
	int pl1fire = -1, pl2fire = -1; //Holds fire situation of players
	int plWait = 0; //Player wait
	int bulWait = 0; //Bullet wait
	int fireWait = 0;

	while ( window->isOpen() ) {
		//-> Here waits are increase according to the speed.
		plWait += speed;
		bulWait += speed;
		fireWait += speed;
		//---

		//-> Scoreboard
		text->setString(to_string(players[1].getScore()) + " - " + to_string(players[0].getScore()));
		text->setPosition((width - text->getLocalBounds().width)/2, height - 2*text->getLocalBounds().height);
		//---

		while (window->pollEvent(event)) {
			//-> Players' variables are set according to the key press.
			if ( event.type == sf::Event::KeyPressed ) { //Takes only keypress event
				switch (event.key.code) {
					case sf::Keyboard::Up:
						pl1move = UP;
						break;
					case sf::Keyboard::Down:
						pl1move = DOWN;
						break;
					case sf::Keyboard::Right:
						pl1move = RIGHT;
						break;
					case sf::Keyboard::Left:
						pl1move = LEFT;
						break;
					case sf::Keyboard::W:
						pl2move = UP;
						break;
					case sf::Keyboard::S:
						pl2move = DOWN;
						break;
					case sf::Keyboard::D:
						pl2move = RIGHT;
						break;
					case sf::Keyboard::A:
						pl2move = LEFT;
						break;
					//-> If pl(x)fire is not -1 then new fire are prevented.
					//   This means user fires just one bullet with the fire key.
					case sf::Keyboard::Enter:
						if ( pl1fire == -1 ) pl1fire = 1;
						break;
					case sf::Keyboard::Space:
						if ( pl2fire == -1 ) pl2fire = 1;
						break;
					//---
					default:
						break;
				}
			//---
			//-> A keyrelease event wipes out its key press event.
			} else if ( event.type == sf::Event::KeyReleased ) { //Takes only keyrelease event
				switch (event.key.code) {
					//-> One player can press two movement keys, then release the old key,
					//   so in this situation soldier should not stop. Here multiple key press
					//   effects are removed with if blocks.
					case sf::Keyboard::Up:
						if (pl1move == UP) pl1move = -1;
						break;
					case sf::Keyboard::Down:
						if (pl1move == DOWN) pl1move = -1;
						break;
					case sf::Keyboard::Right:
						if (pl1move == RIGHT) pl1move = -1;
						break;
					case sf::Keyboard::Left:
						if (pl1move == LEFT) pl1move = -1;
						break;
					case sf::Keyboard::W:
						if (pl2move == UP) pl2move = -1;
						break;
					case sf::Keyboard::S:
						if (pl2move == DOWN) pl2move = -1;
						break;
					case sf::Keyboard::D:
						if (pl2move == RIGHT) pl2move = -1;
						break;
					case sf::Keyboard::A:
						if (pl2move == LEFT) pl2move = -1;
						break;
					//---
					//-> Every user fire just one bullet at any keypress because
					//   if pl(x)fire is not -1 then fire keypress is passed.
					case sf::Keyboard::Enter:
						pl1fire = -1;
						break;
					case sf::Keyboard::Space:
						pl2fire = -1;
						break;
					//---
					default:
						break;
				}
			} else if ( event.type == sf::Event::Closed ) { //Handle the close event.
				window->close();
			}
		}

		//-> Wait section
		if ( bulWait > 2500 ) {
			update();
			bulWait = 0;
		}
		//Fire block does not include update() method it just fires the bullet.
		if ( fireWait > 10000 ) {
			if ( pl1fire == 1 ) {
				players[0].fire(bullets, 18);
				pl1fire = 0;
			}
			if ( pl2fire == 1 ) {
				players[1].fire(bullets + 1, 18);
				pl2fire = 0;
			}
			fireWait = 0;
		}
		if ( plWait > 30000 ) {
			if ( pl1move != -1 ) {
				players[0].walk(18, static_cast<Direction>(pl1move), players, barrels, sandbags, numPlayers, numBarrels, numSandbags);
			}
			if ( pl2move != -1 ) {
				players[1].walk(18, static_cast<Direction>(pl2move), players, barrels, sandbags, numPlayers, numBarrels, numSandbags);
			}
			update();
			plWait = 0;
		}
		//---

		//-> Score check, to decide whether a player is won or not.
		if ( players[0].getScore() >= 10 || players[1].getScore() >= 10 ) {
			//-> Winner text
			if ( players[0].getScore() >= 10 ) {
				text->setString("Player 1 wins,\nstart over? (Y/N)");
			} else if ( players[1].getScore() >= 10 ) {
				text->setString("Player 2 wins,\nstart over? (Y/N)");
			}
			text->setPosition((width - text->getLocalBounds().width)/2, (height - 2*text->getLocalBounds().height)/2);
			//---

			//-> Until a player press y or n keys or until window is closed.
			while ( window->isOpen() ) {
				update();
				window->pollEvent(event);
				//-> If y is pressed, then all entities are reconstructed and reinitialized.
				//   And also default variables are assigned to player variables and wait variables.
				if ( event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Y ) {
					pl1move = -1;
					pl2move = -1;
					pl1fire = -1;
					pl2fire = -1;
					plWait = 0;
					bulWait = 0;
					fireWait = 0;
					//-> Old entities are removed.
					delete [] sandbags;
					delete [] barrels;
					delete [] bullets;
					delete [] players;
					//---
					initEntities();
					break;
				//---
				//-> Else close the window.
				} else if ( (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::N) ||
							(event.type == sf::Event::Closed) ) {
					window->close();
					break;
				}
				//---
			}
			//---
		}
		//---
	}
}

int main(void)
{
	Game shooter(3, 1024, 746, 5,5,2);
	shooter.run2player();
	return 0;
}

