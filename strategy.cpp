#include "strategy.h"

Strategy *NoStrategy::switchMode()
{
    return new AidStrategy(m_balls, m_pockets);
}


CueBall* AidStrategy::findCue(){
    for (int i = 0; i < m_balls->size();i++) {
        Ball* ball = m_balls->at(i);
        if(ball->isCue()){
            CueBall* cue = static_cast<CueBall*> (ball);
            return cue;
        }
    }
    return NULL;
}

void AidStrategy::update(){
    Ball* cue = findCue();
    Ball* candidB = 0;// Candidate Ball
    Pocket* candidP = 0;// Candidate Pocket
    double max_dot = 0; // maximum dot product
    for(int i = 0; i < m_balls->size(); i++){
        Ball* ballA = m_balls->at(i);
        //skip if target ball is cue ball
        if(ballA->isCue()){
            continue;
        }
        //if there are other balls or pockets in between of target ball and cue ball
        if(!checkBall(ballA, cue)){
            continue;
        }
        //find the pocket with smallest angle adjustment
        Pocket* pocket = findPocket(ballA, cue);
        // skip if the angle is over 90 degree
        if(pocket == 0){
            continue;
        }

        //dot product of CA(cue to target) and AP(target to pocket)
        double dot = calculateDot(ballA, cue, pocket);
        if(dot > max_dot){
            candidB = ballA;
            candidP = pocket;
            max_dot = dot;
        }
    }

    if(candidB != 0 && candidP != 0){
        QVector2D posC2 = calculateC2(candidB,cue, candidP);
        toCue = posC2;
        if(toPocket != 0){
            toPocket->revertColour();//change the colour back to balck
        }
        candidP->changeColour();// change the colour to blue
        toPocket = candidP;
    }else if (toCue.length() != 0){
        toCue = QVector2D();
        toPocket->revertColour();
    }
}

void AidStrategy::render(QPainter &painter)
{
    if(toCue.length() != 0){
        //draw the line from cue ball current position to desired position
        Ball* cue = findCue();
        QPen pen = QPen(Qt::DashLine);
        QColor white(Qt::white);
        pen.setColor(white);
        painter.setPen(pen);
        painter.drawLine(cue->getPosition().toPointF(), toCue.toPointF());

        //draw the line for target ball's desired direction
        QVector2D AP = (toPocket->pos() - toCue).normalized()*40;
        QVector2D endPoint = toCue + AP;
        painter.drawLine(toCue.toPointF(), endPoint.toPointF());

        //draw the cue ball in desired position
        white.setAlphaF(0);
        painter.setBrush(QBrush(white));
        painter.drawEllipse(toCue.toPointF(),cue->getRadius(),cue->getRadius());
        painter.setPen(Qt::SolidLine);

    }
}

Strategy *AidStrategy::switchMode()
{
    toPocket->revertColour();
    return new NoStrategy(m_balls, m_pockets);
}

QVector2D AidStrategy::calculateC2 (Ball* ballA, Ball* cue, Pocket* pocket){
    QVector2D posP = pocket->pos();
    QVector2D posA = ballA->getPosition();
    int radiusA = ballA->getRadius();
    int radiusC = cue->getRadius();

    QVector2D line1 = posA - posP;//the target ball to the pocket
    QVector2D line2 = line1.normalized();
    line2 *= radiusA + radiusC;
    QVector2D posC2 = posA + line2; // cue ball desired position
    return posC2;
}

Pocket* AidStrategy::findPocket(Ball* ballA, Ball* cue){
    Pocket* candid = 0;// candidate pocket
    double max_dot = 0;// if the angle is over 90 degree, it's impossible to sink the ball

    for(Pocket* pocket : *m_pockets){
        if(checkPocket(ballA, pocket)){
            double dot = calculateDot(ballA, cue, pocket);
            if(dot > max_dot){
                max_dot = dot;
                candid = pocket;
            }
        }

    }

    return candid;
}

double AidStrategy::calculateDot(Ball* ballA, Ball* cue, Pocket* pocket){
    QVector2D posA = ballA->getPosition();
    QVector2D posC = cue->getPosition();
    QVector2D dirCA = (posA - posC).normalized();// cueball to the target ball

    QVector2D posP = pocket-> pos();
    QVector2D dirAP = (posP - posA).normalized(); // target ball to the pocket
    double dot = QVector2D::dotProduct(dirAP,dirCA);
    return dot;
}

bool AidStrategy::checkBall(Ball* ballA, Ball* cue){
    bool noCllide = true;
    QVector2D posA = ballA->getPosition();
    QVector2D posC = cue->getPosition();
    int radiusC = cue->getRadius();
    //check the intersection wit hother balls
    for(int i = 0; i < m_balls->size(); i++){
        Ball* ballB = m_balls->at(i);
        if(ballB == ballA || ballB == cue){
            continue;
        }
        QVector2D posB = ballB->getPosition();// other ball's position
        if(minimum_distance(posA, posC, posB) <= radiusC + ballB->getRadius()){
            noCllide = false;
            break;
        }
    }
    //check if the cue ball will sink before hit the target ball
    for(Pocket* pocket : *m_pockets){
        QVector2D posP = pocket->pos();
        if(minimum_distance(posA, posC, posP) <= pocket->radius() - radiusC){
            noCllide = false;
            break;
        }
    }
    return noCllide;
}

bool AidStrategy::checkPocket(Ball* ballA, Pocket* pocket){
    bool noCllide = true;
    QVector2D posA = ballA->getPosition();
    QVector2D posP = pocket-> pos();
    int radiusA = ballA->getRadius();
    if(pocket->radius() < radiusA){
        return false;
    }

    //check the if there's any ball between the target ball and the pocket
    for(int i = 0; i < m_balls->size(); i++){
        Ball* ballB = m_balls->at(i);
        if(ballB == ballA){
            continue;
        }

        QVector2D posB = ballB->getPosition();
        if(minimum_distance(posA, posP, posB) <= radiusA + ballB->getRadius()){
            noCllide = false;
            break;
        }
    }
    return noCllide;
}

//Obtained from https://stackoverflow.com/a/1501725
float AidStrategy::minimum_distance(QVector2D v, QVector2D w, QVector2D p) {
  // Return minimum distance between line segment vw and point p
  const float l2 = (w-v).lengthSquared();  // i.e. |w-v|^2 -  avoid a sqrt
  if (l2 == 0.0) return p.distanceToPoint(v);   // v == w case
  // Consider the line extending the segment, parameterized as v + t (w - v).
  // We find projection of point p onto the line.
  // It falls where t = [(p-v) . (w-v)] / |w-v|^2
  // We clamp t from [0,1] to handle points outside the segment vw.
  const float t = fmax(0, fmin(1, QVector2D::dotProduct(p - v, w - v) / l2));
  const QVector2D projection = v + t * (w - v);  // Projection falls on the segment
  return p.distanceToPoint(projection);
}

