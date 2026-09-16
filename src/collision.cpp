#include "collision.hpp"
namespace moss {
bool blocked(const Region& m,Vec p,float r) {
    int left=static_cast<int>(std::floor((p.x-r)/Tile)),right=static_cast<int>(std::floor((p.x+r)/Tile));
    int top=static_cast<int>(std::floor((p.y-r)/Tile)),bottom=static_cast<int>(std::floor((p.y+r)/Tile));
    for(int y=top;y<=bottom;++y) for(int x=left;x<=right;++x) if(m.solid(x,y)) return true;
    return false;
}
void moveBody(const Region& m,Vec& p,Vec d,float r) {
    int steps=std::max(1,static_cast<int>(std::ceil(length(d)/3)));
    d=d*(1.0f/steps);
    for(int i=0;i<steps;++i) {
        if(!blocked(m,{p.x+d.x,p.y},r)) p.x+=d.x;
        if(!blocked(m,{p.x,p.y+d.y},r)) p.y+=d.y;
    }
}
bool lineClear(const Region& m,Vec a,Vec b) {
    int steps=std::max(1,static_cast<int>(std::ceil(length(b-a)/4)));
    for(int i=0;i<=steps;++i) if(blocked(m,a+(b-a)*(float(i)/steps),0)) return false;
    return true;
}
Vec safePosition(const Region& m,Vec p) {
    if(!blocked(m,p)) return p;
    Vec best=tileCenter(2,2); float distance=1e9f;
    for(int y=1;y<m.height-1;++y) for(int x=1;x<m.width-1;++x) {
        Vec candidate=tileCenter(x,y); float d=length(candidate-p);
        if(!blocked(m,candidate)&&d<distance) { distance=d; best=candidate; }
    }
    return best;
}
}
