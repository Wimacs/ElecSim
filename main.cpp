
#include <SFML\Graphics.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include "Utility.h"
#include "Point.h"
#include "Charge.h"
#include "Button.h"
#include "Slider.h"

#define SCR_WIDTH 1100
#define SCR_HEIGHT 900

#define MENU_WIDTH 250

#define SGMT_LENGTH 10
#define SGMT_NB 100

#define E_FIELD_NUMBER 50
#define V_FIELD_NUMBER 300

int main()
{
	RenderWindow window(VideoMode(SCR_WIDTH,SCR_HEIGHT),"ElectroStat") ; //Start the Whole APP
	window.setFramerateLimit(120) ;

	vector<Charge> charges ; // field lines
	VertexArray EFieldVA(Lines) ; 
	vector<VertexArray> EFieldLinesVA(LinesStrip) ;
	vector<float> VField ;
	
	vector<Vector2f> wallPoints ; // walls
	wallPoints.push_back(Vector2f(MENU_WIDTH,0)) ;
	wallPoints.push_back(Vector2f(MENU_WIDTH+1,SCR_HEIGHT)) ;
	wallPoints.push_back(Vector2f(MENU_WIDTH+1,SCR_HEIGHT)) ;
	wallPoints.push_back(Vector2f(SCR_WIDTH,SCR_HEIGHT+1)) ;
	wallPoints.push_back(Vector2f(SCR_WIDTH,SCR_HEIGHT+1)) ;
	wallPoints.push_back(Vector2f(SCR_WIDTH-1,0)) ;
	wallPoints.push_back(Vector2f(SCR_WIDTH-1,0)) ;
	wallPoints.push_back(Vector2f(MENU_WIDTH,0)) ;
	
	RectangleShape menuBackground ; menuBackground.setSize(Vector2f(MENU_WIDTH,SCR_HEIGHT)) ; menuBackground.setFillColor(Color::Color(210,210,210)) ; //menu
	menuBackground.setOutlineColor(Color::Black) ; menuBackground.setOutlineThickness(5) ; 
	Font arial ; arial.loadFromFile("arial.ttf") ;
	
	Button EFieldButton(Vector2f(20,140), Vector2f(20,20), arial, "Draw Vector Field : ", Color::Yellow, false) ;
	
	Button drawNegativeLinesButton(Vector2f(20,170), Vector2f(20,20), arial, "Draw op Lines : ", Color::Yellow, true) ;
	Button movingChargeButton(Vector2f(20,200), Vector2f(20,20), arial, "Movable : ", Color::Yellow, true) ;
	Button EFieldLine(Vector2f(20,230), Vector2f(20, 20), arial, "Draw Lines : ", Color::Yellow, true);
		
	Slider chargeSlider(Vector2f(100,10),Vector2f(10,20),Vector2f(20,20),-10,10,"Charge : ",arial) ;
	Slider massSlider(Vector2f(100,10),Vector2f(10,20),Vector2f(20,50),1,1000,"Mass : ",arial) ;
	Slider fieldLinesSlider(Vector2f(100,10),Vector2f(10,20),Vector2f(20,80),0,30,"Quantity: ",arial) ;
	Slider VQualitySlider(Vector2f(100,10),Vector2f(10,20),Vector2f(20,110),0,6,"Quanlity : ",arial) ;

	Clock deltaTime ; // time
	Clock time ;
	float dt=0.001 ;

	Image image ;
	image.create(SCR_WIDTH-MENU_WIDTH,SCR_HEIGHT,Color::Black) ;
	Texture texture ;
	texture.setSmooth(true);
	Sprite sprite(texture) ; sprite.setPosition(MENU_WIDTH,0) ;
	
	bool mouseClickedLastFrame = false ; // mouse

	while(window.isOpen())
	{
		Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

		Vector2f mousePos( Mouse::getPosition(window).x, Mouse::getPosition(window).y) ;

		EFieldButton.isButtonClicked(Mouse::Left, mousePos, mouseClickedLastFrame) ;
		movingChargeButton.isButtonClicked(Mouse::Left, mousePos, mouseClickedLastFrame) ;
		drawNegativeLinesButton.isButtonClicked(Mouse::Left, mousePos, mouseClickedLastFrame) ;
		chargeSlider.isSliderClicked(Mouse::Left, mousePos) ;
		massSlider.isSliderClicked(Mouse::Left, mousePos) ;
		fieldLinesSlider.isSliderClicked(Mouse::Left, mousePos) ;
		VQualitySlider.isSliderClicked(Mouse::Left, mousePos) ;
		EFieldLine.isButtonClicked(Mouse::Left, mousePos, mouseClickedLastFrame);

		if(mouseClickedOnce(Mouse::Left, mouseClickedLastFrame))
		{
			if(mousePos.x>MENU_WIDTH)
			{
				Charge c(chargeSlider.getSliderValue(), massSlider.getSliderValue(), mousePos) ;
				if(!movingChargeButton.getCondition())
					c.setStatic() ;
				charges.push_back(c) ;
			}
		}
		
		for(int i(0) ; i < charges.size() ; i++)
		{
			Vector2f previousPosition = charges[i].getPosition() ;
			if(charges[i].isMovable())
				charges[i].updatePosition(dt) ;
			charges[i].clearForce() ;
			for(int j(0) ; j < charges.size() ; j++)
			{
				if(j!=i)
					charges[i].addForce(charges[i].getCoulombForce(charges[j])) ;
			}
			for(int j(0) ; j < wallPoints.size() ; j+=2)
			{
				if(determinant(wallPoints[j+1] - wallPoints[j], wallPoints[j+1]-charges[i].getPosition())*determinant(wallPoints[j+1] - wallPoints[j], wallPoints[j+1]-previousPosition)<0) 
				{
					Vector2f v = wallPoints[j+1] - wallPoints[j] ;
					Vector2f n = Vector2f(-v.y,v.x) ;             
					n/=Norm(n) ;
					charges[i].updatePosition(-dt) ;
					float j = -(1+0.5)*dotProduct(charges[i].getSpeed(), n) ;

					charges[i].setSpeed(charges[i].getSpeed() + j*n) ;
				}
			}
		}
		//cout<<Charge::getV(mousePos,charges)<<endl ;
		window.clear(Color::White) ; ///////////////////// DRAW /////////////////////////
		float quality = VQualitySlider.getSliderValue() ;
		if(quality != 0)
		{
			for(int x(0) ; x < SCR_WIDTH-MENU_WIDTH ; x+=quality)
			{
				for(int y(0) ; y < SCR_HEIGHT ; y+=quality)
				{
					float V = Charge::getV(Vector2f(x+MENU_WIDTH,y),charges)*10 ;
					for(int i(0) ; i < quality ; i++)
					{
						for(int j(0) ; j < quality ; j++)
						{
							image.setPixel(x+i,y+j,Color::Color(0,0,0)) ;
						}
					}
					
				}
			}
			texture.loadFromImage(image) ;
			sprite.setTexture(texture,true) ;
			window.draw(sprite) ;
		}

		if(EFieldButton.getCondition())
		{
			float d = SCR_HEIGHT/E_FIELD_NUMBER ;
			for(int x(0) ; x < E_FIELD_NUMBER+1 ; x++)
			{
				for(int y(0) ; y < E_FIELD_NUMBER+1 ; y++)
				{
					Vertex v1(Vector2f(MENU_WIDTH+d*x,d*y)) ; v1.color = Color(0,clamp(Norm(Charge::getEField(v1.position,charges,1000)),0,255),0);
					Vertex v2(Vector2f(MENU_WIDTH+d*x,d*y)+ normalized(Charge::getEField(Vector2f(MENU_WIDTH+d*x,d*y),charges,1000))) ; 
					v2.color = Color(0, 0, 0);
					EFieldVA.append(v1) ;
					EFieldVA.append(v2) ;
				
				}
			}
			window.draw(EFieldVA) ;
		}
		
		if (EFieldLine.getCondition())
		{
			for (int i(0); i < charges.size(); i++)
			{
				if (!drawNegativeLinesButton.getCondition() && charges[i].getCharge() > 0)
				{
					EFieldLinesVA = charges[i].getFieldLineVA(fieldLinesSlider.getSliderValue(), charges, Color::White, SGMT_LENGTH, SGMT_NB);
					for (int j(0); j < EFieldLinesVA.size(); j++)
					{
						window.draw(EFieldLinesVA[j]);
					}
				}
				else if (drawNegativeLinesButton.getCondition())
				{
					EFieldLinesVA = charges[i].getFieldLineVA(fieldLinesSlider.getSliderValue(), charges, Color::White, SGMT_LENGTH, SGMT_NB);
					for (int j(0); j < EFieldLinesVA.size(); j++)
					{
						window.draw(EFieldLinesVA[j]);
					}
				}

			}
		}
		
		
		for(int i(0) ; i < charges.size() ; i++)
		{
			if(charges[i].getCharge()>0)
				drawPoint(charges[i].getPosition(),5,window,Color::Red) ;
			else
				drawPoint(charges[i].getPosition(),5,window,Color::Blue) ;
		}
		

		window.draw(menuBackground) ;

		EFieldButton.draw(window) ;
		EFieldLine.draw(window);
		movingChargeButton.draw(window) ;
		drawNegativeLinesButton.draw(window) ;
		chargeSlider.draw(window) ;
		massSlider.draw(window) ;
		fieldLinesSlider.draw(window) ;
		VQualitySlider.draw(window) ;


		window.display() ;

		EFieldVA.clear() ;
		EFieldLinesVA.clear() ;
		
		dt = deltaTime.restart().asSeconds() ;
	}
}

