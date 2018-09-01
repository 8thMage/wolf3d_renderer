function generator
disp('Welcome to the random porous medium generator')
  prompt = 'please select length in mm (10-50) : ';
LX = input(prompt);
prompt = 'please select width in mm (10-50) : ';
LY = input(prompt);
prompt = 'please select porosity (0.5-0.9) : ';
por = input(prompt);
space_x=sqrt(LX);
space_y=sqrt(LY);
answer=1;
while answer==1
  N=0;
a=1;
block=ones(LY,LX);
k=1;
for i=linspace(2,LY-1,space_y)
  for j=linspace(2,LX-1,space_x)
    points(k,1)=floor(i);
points(k,2)=floor(j);
k=k+1;
end
end
while a==1
  for k=1:length(points)
    if block(points(k,1),points(k,2))==1
  block(points(k,1),points(k,2))=0;
N=N+1;
end
n=N/(LX*LY-(2*LX+2*LY)+4);
if n>por
a=0;
end
b=1;
while b==1
  num=choose_rand(3)-2;
d=choose_rand(2);
if d==1
  points(k,1)=points(k,1)+num;
end
if d==2
  points(k,2)=points(k,2)+num;
end
if block(points(k,1),points(k,2))==1;
b=0;
end
end
if points(k,1)==1
  points(k,1)=2;
end
if points(k,2)==1
  points(k,2)=2;
end
if points(k,1)==LY
  points(k,1)=LY-1;
end
if points(k,2)==LX
  points(k,2)=LX-1;
end
end
end
block(1,:)=1;
block(LY,:)=1;
block(:,1)=2;
block(:,LX)=3;
fid = fopen('domain.txt', 'w');
fprintf(fid, '%i \n', LX);
fprintf(fid, '%i \n', LY);
for i=1:LY
  for j=1:LX
  fprintf(fid, '%i \n', block(i,j));
end
end
fclose(fid);
hold on
colormap([1 1 1; 0 0 0 ]);
image(block .* 255);
axis([2 LX-1 1 LY]);
prompt = 'Would you like to try again? (0 = no, 1 = yes) ';
answer = input(prompt);
if answer == 1
  prompt = 'Would you like to try a different porosity? (0 = no, 1 = yes) ';
answer_1 = input(prompt);
if answer_1 == 1
  prompt = 'please select new porosity (0.5-0.9) : ';
por = input(prompt);
end
end
end

function [num]=choose_rand(N)
  n=rand(1);
for x=0:(N-1)
  if (n>=x/N)&&(n<=(x+1)/N)
    num=x+1;
end
end
end

end
