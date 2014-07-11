s=serial('COM4', 'BaudRate', 9600, 'Parity','none');
set(s, 'InputBufferSize', 6);
fopen(s);

fprintf(s, '$');

%figure(1)

count = 0;
A = [0 0 0 0];
output = [];
xaxis = [];

while(count < 200)
%    disp(count)
    bytes = s.BytesAvailable;

    if ~isempty(s.BytesAvailable)
        if s.BytesAvailable~=0
            count = count+1;
            a=fread(s, 6);
            %   a=fread(s, s.BytesAvailable);
            flushinput(s);
            index = find(a==13);
            for i=1:4
                if index==i
                    A(i) = a(6);
                else
                    A(i) = a(mod(index-i, 6));
                end
            end

            for i=1:4
                if A(i)~=0
                    A(i) = A(i)-48;
                end
            end

            xaxis(count) = count;
            output(count) = A(1) + A(2)*10 + A(3)*100 + A(4)*1000;
            disp(output(count))
            plot(xaxis, output)
            axis([1 200 0 1023])
            hold on

        end
        
    end
    pause(0.01)
end

hold off
fclose(s);
delete(s);
clear s;
