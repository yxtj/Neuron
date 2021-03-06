
addpath('../mBasic/')
basicDataParameters

function data=loadNeuronData(fnlist,fun)
  nTri=length(fnlist);
  if(nTri==0) return; end;
  data=readRaw(fnlist(1))(:);
  nNeu=length(data);
  t=cell(nNeu,nTri);
  t(:,1)=data;
  if(nargin<2)%default: no pre-process
    for i=2:nTri;  t(:,i)=readRaw(fnlist(i))(:);  end;
  else
    for i=2:nTri;  t(:,i)=fun(readRaw(fnlist(i))(:));  end;
  end
  for i=1:nNeu;
    data(i)=cell2mat(t(i,:));
  end
end;

################
#try possion process (intervals follow exponential distribution)

%load interval data
function info=diffFun(data)
  nNeu=numel(data);
  info=cell(size(data));
  for i=1:nNeu;
    t=cell2mat(data(i));
    if(length(t)>=2) info(i)=t(2:end)-t(1:end-1); end;
  end
end
data=cell(nNeu,nCue);
for i=1:nCue
  data(:,i)=loadNeuronData(fnlist(:,i),@diffFun);
end
cid=1;
data2=cell(nNeu,nTri);
for i=1:nTri
  data2(:,i)=loadNeuronData(fnlist(i,cid),@diffFun);
end


hist(cell2mat(data(1)),100);
for i=1:9;
  nid=randi(nNeu,9,1);cid=randi(nCue,9,1);
  subplot(3,3,i);hist(cell2mat(data(nid(i),cid(i))),100);
  title(['N',num2str(nid(i)),'-C',num2str(cid(i))]);ylabel('frequency');
end

%hypothesis testing (Kolmogorov–Smirnov method)
%step 1, estimate exponential parameter -- lambda (required by step 2)

lambda=zeros(nNeu,nCue);
for i=1:nNeu;for j=1:nCue;
  lambda(i,j)=mean(cell2mat(data(i,j)));
end;end
lambda2=zeros(nNeu,nTri);
for i=1:nNeu;for j=1:nTri;
  lambda2(i,j)=mean(cell2mat(data2(i,j)));
end;end

%step 2, distribution testing

pval=zeros(nNeu,nCue);
for i=1:nNeu;for j=1:nCue;
  pval(i,j)=kolmogorov_smirnov_test(cell2mat(data(i,j)),'exp',lambda(i,j));
end;end
plot(pval)
%pval>alpha -> accept; pval<=alpha -> reject(not exponential on lambda)
pval2=zeros(nNeu,nCue);
for i=1:nNeu;for j=1:nTri;
  t=cell2mat(data2(i,j));
  if(isempty(t))  pval2(i,j)=-1; continue;  end;
  pval2(i,j)=kolmogorov_smirnov_test(t,'exp',lambda2(i,j));
end;end
