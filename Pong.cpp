#include "stdafx.h"
#include <iostream>
#include "Pong.h"

Pong::Pong() {
	playerOneScore = 0;
	playerTwoScore = 0;
	screenWidth = 800;
	screenHeight = 800;
	paddleSize.x = 10;
	paddleSize.y = 100;
	paddleSpeed = 800;
	paused = false;

	window.create(sf::VideoMode(screenWidth, screenHeight), "Pong");
	LoadFont();
	LoadSounds();
	CreatePaddles();
	CreateBall();
	CreateBorders();
	CreateText();
}

void Pong::LoadFont() {
	if (!font.loadFromFile("assets/lucon.ttf")) {
		std::cout << "Error loading font\n";
		exit(EXIT_FAILURE);
	}
	std::cout << "Font loaded correctly\n";
}

void Pong::LoadSounds() {
	if (!collideBuffer.loadFromFile("assets/collide.wav") || !scoreBuffer.loadFromFile("assets/score.wav") || !pauseBuffer.loadFromFile("assets/pause.wav") || !music.openFromFile("assets/music.wav")) {
		std::cout << "Error loading one or more sounds\n";
		exit(EXIT_FAILURE);
	}

	collide.setBuffer(collideBuffer);
	score.setBuffer(scoreBuffer);
	pause.setBuffer(pauseBuffer);
	music.setLoop(true);
	music.play();
	std::cout << "Sounds loaded correctly\n";
}

void Pong::CreatePaddles() {
	playerOnePaddle = Paddle(sf::Keyboard::Key::W, sf::Keyboard::Key::S, paddleSize, sf::Color::Red, sf::Color::White, 3, sf::Vector2f(50, screenWidth / 2), sf::Vector2f(paddleSize.x / 2, paddleSize.y / 2));
	playerTwoPaddle = Paddle(sf::Keyboard::Key::Up, sf::Keyboard::Key::Down, paddleSize, sf::Color::Blue, sf::Color::White, 3, sf::Vector2f(screenWidth - 50, screenWidth / 2), sf::Vector2f(paddleSize.x / 2, paddleSize.y / 2));
}

void Pong::CreateBall() {
	pongBall = Ball(30, 10, 0, sf::Color::White, sf::Vector2f(screenWidth / 2, screenHeight / 2));
}

void Pong::CreateBorders() {
	ceiling = Wall(sf::Vector2f(screenWidth, paddleSize.x), sf::Color::Magenta, sf::Vector2f(0, 0));
	floor = Wall(sf::Vector2f(screenWidth, paddleSize.x), sf::Color::Magenta, sf::Vector2f(0, screenHeight - paddleSize.x));
}

void Pong::CreateText() {
	playerOneText = Text("Player 1: " + std::to_string(playerOneScore), sf::Color::White, 20, sf::Vector2f(65, 50));
	playerOneText.setFont(font); //SFML doesn't play nice with fonts assigned in different classes, set font locally for now
	playerTwoText = Text("Player 2: " + std::to_string(playerTwoScore), sf::Color::White, 20, sf::Vector2f(screenWidth - 200, 50));
	playerTwoText.setFont(font);
	pauseText = Text("P A U S E D", sf::Color::White, 50, sf::Vector2f((screenWidth / 2) - 165, (screenHeight / 2) - 30));
	pauseText.setFont(font);
}

void Pong::RegisterScores(const int &player) {
	switch (player) {
	case 1:
		playerOneScore++;
		playerOneText.setString("Player 1: " + std::to_string(playerOneScore));
		pongBall.SetAngle(0); //Serve to Player 2
		break;
	case 2:
		playerTwoScore++;
		playerTwoText.setString("Player 2: " + std::to_string(playerTwoScore));
		pongBall.SetAngle(180); //Serve to Player 1
		break;
	default:
		break;
	}
	score.play();
	pongBall.setPosition(sf::Vector2f(screenWidth / 2, screenHeight / 2)); //Reset Ball to centre
}

void Pong::Render() {
	window.draw(playerOnePaddle);
	window.draw(playerTwoPaddle);
	window.draw(playerOneText);
	window.draw(playerTwoText);
	window.draw(ceiling);
	window.draw(floor);
	window.draw(pongBall);
	window.display();
}

int Pong::Run() {
	while (window.isOpen())
	{
		float time = clock.restart().asSeconds();
		float factor = time * 400; //Determines speed of Ball
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed || ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape)))
				window.close();
			if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Space)) {
				pause.play();
				paused = !paused;
			}
		}

		if (!paused) { //PLAYING
			pongBall.MoveBall(factor); // Ball Movement
			
			//CONTROLS
			//Player 1 Controls
			playerOnePaddle.ReadInput(ceiling, floor, paddleSpeed * time);
			////Player 2 Controls
			playerTwoPaddle.ReadInput(ceiling, floor, paddleSpeed * time);

			//SCORING
			//Player 1 Score
			if (pongBall.getPosition().x > playerTwoPaddle.getPosition().x + 50) {
				RegisterScores(1);
			}
			//Player 2 Score
			if (pongBall.getPosition().x < playerOnePaddle.getPosition().x - 50) {
				RegisterScores(2);
			}
			
			//BALL COLLISION
			//Borders
			if (pongBall.getGlobalBounds().intersects(ceiling.getGlobalBounds()) || pongBall.getGlobalBounds().intersects(floor.getGlobalBounds())) {
				pongBall.InverseAngle();
			}
			//Player One Paddle
			if (pongBall.getGlobalBounds().intersects(playerOnePaddle.getGlobalBounds()) && cos(pongBall.GetAngle()) <= 0) { //cos(ballAngle) < 0 checks that the ball is coming from the right before reflecting
				collide.play();
				pongBall.BallPaddleCollision(playerOnePaddle);
			}
			//Player Two Paddle
			if (pongBall.getGlobalBounds().intersects(playerTwoPaddle.getGlobalBounds()) && cos(pongBall.GetAngle()) >= 0) { //cos(ballAngle) > 0 checks that the ball is coming from the left before reflecting
				collide.play();
				pongBall.BallPaddleCollision(playerTwoPaddle);
			}

		}
		window.clear();

		if(paused) {
			window.draw(pauseText);
		}

		Render();
	}

	return 0;
}
