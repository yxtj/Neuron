function dp=cal_multiple_real(x,L,r,window_size,nSample=100,type='DP')
  dp=zeros(length(window_size),1);
  for i=1:nSample;
    x=find(x<=L);n=length(x);
    rnd=unifrnd(-r,r,n,1);%uniform
%    rnd=normrnd(0,r/3,n,1);%normal
    y=max(0,x+rnd);
    for wid=1:length(window_size);
      ws=window_size(wid);
      if(strcmp(type,'DP'))
        dx=discretize(x,ws,0,ceil(L/ws),'binary');
        dy=discretize(y,ws,0,ceil(L/ws),'binary');
        dp(wid)+=dot_product(dx,dy);
      else; %MI
        dx=discretize(x,ws,0,ceil(L/ws),'count');
        dy=discretize(y,ws,0,ceil(L/ws),'count');
        dp(wid)+=mutual_info(dx,dy);
      end
    end
  end;
  dp/=nSample;
end;
